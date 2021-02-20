
#include "terminal.h"
#include "common.h"
#include "log.h"
#include "cfg2.h"
#include "str.h"

#ifdef LINUX
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#endif

string_t g_consoleCommand;
string_t *g_commandHistory;
string_t g_commandComplete;

int terminal_getHistoryLine(string_t *line, int *index);

int terminal_terminalInit(void) {
	int error = ERR_CRITICAL;
	
#ifdef LINUX
	struct termios config;

	if (!isatty(STDIN_FILENO)) {
		critical_error("Not a TTY. Shouldn't happen.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}

	if (tcgetattr(STDIN_FILENO, &config) < 0) {
		critical_error("Can't get TTY. Shouldn't happen.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	// config.c_iflag = INLCR;
	// config.c_oflag = ONLCR;
	// config.c_cflag = 0;
	// config.c_lflag = 0;
	
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	
	config.c_cflag &= ~(CSIZE | PARENB);
	config.c_cflag |= CS8;

	config.c_cc[VMIN]  = 1;
	config.c_cc[VTIME] = 0;
	
	if (cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
		critical_error("Couldn't set baud rate. Shouldn't happen.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &config) < 0) {
		critical_error("Terminal configuration error. Shouldn't happen.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	
#endif
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int terminal_callback_updateCommandHistoryLength(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	static int lastLength = CFG_HISTORY_LENGTH_DEFAULT;
	int length = var->integer;
	
	if (length <= 0) {
		var->integer = 1;
		length = var->integer;
		warning(CFG_HISTORY_LENGTH" is not a whole number. Setting to %i.", var->integer);
	}
	
	if (length != lastLength) {
	
		if (length < lastLength) {
			// Need to free.
			for (int i = length; i < lastLength; i++) {
				string_free(&g_commandHistory[i]);
			}
		}
	
		g_commandHistory = realloc(g_commandHistory, length * sizeof(string_t));
		if (g_commandHistory == NULL) {
			critical_error("Out of memory", "");
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		if (length > lastLength) {
			// Need to allocate.
			for (int i = lastLength; i < length; i++) {
				error = string_init(&g_commandHistory[i]);
				if (error) {
					critical_error("Out of memory", "");
					goto cleanup_l;
				}
			}
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	lastLength = length;
	
	return error;
}

// @TODO: This is horrible! Rewrite!! FIXME!!! -- Is this still the case? Commands have been completely removed in this system.
/* terminal_fragmentCompletion
Completes a command fragment. badComplete is set if multiple options have been
found. Tabs allow selection when multiple options are available.
fragment:       The partial command that is to be completed.
badComplete:    True if there are multiple completions.
tabs:           The number of times the user has pressed <tab> so far.
*/
static int terminal_fragmentCompletion(string_t *fragment, bool *badComplete, int *tabs) {
	int error = ERR_CRITICAL;

	bool variablePotentials[g_cfg2.vars_length];
	int variablePotentialsCount = 0;
	int tabIndex = 0;
	const int maxPotentials = 10;
	int potentialsLoopCounter = 0;
	
	// Find matching variables.
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		variablePotentials[i] = !strncmp(fragment->value, g_cfg2.vars[i].name, fragment->length);
		if (variablePotentials[i]) {
			variablePotentialsCount++;
		}
	}
	
	if (variablePotentialsCount > maxPotentials) {
		variablePotentialsCount = maxPotentials;
	}
	
	if (variablePotentialsCount > 1) {
		*badComplete = 1;
		putc('\n', stdout);
	}
	
	if (*tabs >= variablePotentialsCount) {
		*tabs = 0;
	}
	
	for (int i = 0; (i < g_cfg2.vars_length) && (potentialsLoopCounter < variablePotentialsCount); i++) {
		// One correction
		if ((variablePotentialsCount == 1) && (variablePotentialsCount == 1)) {
			error = string_copy_c(fragment, g_cfg2.vars[i].name);
			if (error) {
				goto cleanup_l;
			}
			break;
		}
		// Multiple corrections
		else {
			if (tabIndex == *tabs) {
				string_copy_c(&g_commandComplete, g_cfg2.vars[i].name);
				printf(COLOR_BLACK B_COLOR_WHITE"%s"COLOR_NORMAL"\n", g_cfg2.vars[i].name);
			}
			else {
				puts(g_cfg2.vars[i].name);
			}
			tabIndex++;
		}
		potentialsLoopCounter++;
	}
	
	if (variablePotentialsCount > 1) {
		printf("> ");
		fflush(stdout);
	}
	else {
		string_copy_c(&g_commandComplete, "");
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

/* terminal_codeCompletion
Finds an incomplete variable in a command and attempts to complete it.
line:           The line with an incomplete variable.
badComplete:    True if there are multiple completions.
tabs:           The number of times the user has pressed <tab> so far.
*/
static int terminal_codeCompletion(string_t *line, bool *badComplete, int *tabs) {
	int error = ERR_CRITICAL;
	char *start, *space;
	int length;
	string_t fragment;
	
	error = string_init(&fragment);
	if (error) {
		goto cleanup_l;
	}
	
	start = line->value;
	space = NULL;
	
	// Find the last variable on the line. The variable is probably incomplete.
	while (1) {
		space = strchr(start, ' ');
		if (space == NULL) {
			break;
		}
		start = space + 1;
	}
	
	length = line->value + line->length - start;
	
	// Copy the last variable into "fragment".
	string_copy_length_c(&fragment, start, length);
	
	// Do command complete on the fragment.
	if (space == NULL) {
		error = terminal_fragmentCompletion(&fragment, badComplete, tabs);
		if (error) {
			goto cleanup_l;
		}
	}
	
	// Stick the completed variable back on the end of the line.
	start[0] = '\0';
	error = string_normalize(line);
	if (error) {
		goto cleanup_l;
	}
	error = string_concatenate(line, &fragment);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	string_free(&fragment);
	
	return error;
}

/* terminal_enterCompletion
Complete the fragment and replace it with the user-selected variable.
*/
static int terminal_enterCompletion(string_t *line) {
	int error = ERR_CRITICAL;
	char *start, *space;
	
	start = line->value;
	space = NULL;
	
	while (1) {
		space = strchr(start, ' ');
		if (space == NULL) {
			break;
		}
		start = space + 1;
	}
	
	start[0] = '\0';
	error = string_normalize(line);
	if (error) {
		goto cleanup_l;
	}
	error = string_concatenate(line, &g_commandComplete);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int terminal_runTerminalCommand(void) {
	int error = ERR_CRITICAL;

	static bool printedPrompt = false;
	static bool escape = false;
	static bool controlSequence = false;
	static bool insert = true;
	static int cursor = 0;
	static int historyIndex = -1;
	char character;
	bool badComplete = false;
	static int tabs = 0;
	int tempInt;
	cfg2_admin_t tempAdmin;
	
	if (!printedPrompt) {
		printedPrompt = true;
		printf("> ");
		fflush(stdout);
	}
	
#ifdef LINUX
	if (read(STDIN_FILENO, &character, 1) > 0) {
#else
#error "Must rewrite for non-Linux platforms"
	{
#endif
		if (controlSequence) {
			switch (character) {
			// Cursor up (Up arrow) → Up in history
			case 'A':
				historyIndex++;
				terminal_getHistoryLine(&g_consoleCommand, &historyIndex);
				
				while (cursor < g_consoleCommand.length) {
					putc('#', stdout);
					cursor++;
				}
				while (cursor > 0) {
					putc('\b', stdout);
					putc(' ', stdout);
					putc('\b', stdout);
					--cursor;
				}
				while (cursor < g_consoleCommand.length) {
					putc(g_consoleCommand.value[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			// Cursor down (Down arrow) → Down in history
			case 'B':
				--historyIndex;
				terminal_getHistoryLine(&g_consoleCommand, &historyIndex);
				
				while (cursor < g_consoleCommand.length) {
					putc('#', stdout);
					cursor++;
				}
				while (cursor > 0) {
					putc('\b', stdout);
					putc(' ', stdout);
					putc('\b', stdout);
					--cursor;
				}
				while (cursor < g_consoleCommand.length) {
					putc(g_consoleCommand.value[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			// Cursor forward (Right arrow)
			case 'C':
				if (cursor < g_consoleCommand.length) {
					putc(g_consoleCommand.value[cursor], stdout);
					fflush(stdout);
					cursor++;
				}
				break;
			// Cursor back (Left arrow)
			case 'D':
				if (cursor > 0) {
					--cursor;
					putc('\b', stdout);
					fflush(stdout);
				}
				break;
			default:;
			}
			
			controlSequence = false;
			error = ERR_OK;
			goto cleanup_l;
		}
		
		if (escape) {
			switch (character) {
			case '[':
				controlSequence = true;
				break;
			default:;
			}
			
			escape = false;
			error = ERR_OK;
			goto cleanup_l;
		}
		
		// Do action for key.
		switch (character) {
		// Enter → Execute command.
		case '\r':
			// Do final tab completion.
			if ((tabs > 0) && (g_commandComplete.length != 0)) {
				tabs = 0;
				error = terminal_enterCompletion(&g_consoleCommand);
				if (error) {
					goto cleanup_l;
				}
				// cursor = 0;
				while (cursor < g_consoleCommand.length) {
					// You should never see this.
					putc('#', stdout);
					cursor++;
				}
				while (cursor > 0) {
					putc('\b', stdout);
					putc(' ', stdout);
					putc('\b', stdout);
					--cursor;
				}
				while (cursor < g_consoleCommand.length) {
					putc(g_consoleCommand.value[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			}
		
			putc('\n', stdout);
			
			// // Execute string as administrator.
			// g_cfg.lock = false;
			tempAdmin = g_cfg2.adminLevel;
			g_cfg2.adminLevel = cfg2_admin_administrator;
			error = cfg2_execString(&g_consoleCommand, "console", 0);
			g_cfg2.adminLevel = tempAdmin;
			// g_cfg.lock = true;
			if (error == ERR_OUTOFMEMORY) {
				critical_error("Out of memory", "");
				goto cleanup_l;
			}
			if (error == ERR_CRITICAL) {
				goto cleanup_l;
			}
			
			printedPrompt = false;
			historyIndex = -1;
			cursor = 0;
			g_consoleCommand.length = 0;
			break;
		// Backspace
		case '\x7F':
			if (cursor > 0) {
				for (int i = cursor; i < g_consoleCommand.length; i++) {
					g_consoleCommand.value[i - 1] = g_consoleCommand.value[i];
				}
				g_consoleCommand.value[g_consoleCommand.length - 1] = ' ';
				
				tempInt = cursor;
				--cursor;
				putc('\b', stdout);
				
				while (cursor < g_consoleCommand.length) {
					putc(g_consoleCommand.value[cursor], stdout);
					cursor++;
				}
				
				--tempInt;
				while (cursor > tempInt) {
					--cursor;
					putc('\b', stdout);
				}
				
				fflush(stdout);
				
				--g_consoleCommand.length;
				g_consoleCommand.value[g_consoleCommand.length] = '\0';
			}
			break;
		// ESC
		case '\x1B':
			escape = true;
			break;
		// ^C → Cease conjuring
		case '\x03':
			putc('\n', stdout);
			g_consoleCommand.length = 0;
			g_consoleCommand.value[0] = '\0';
			cursor = 0;
			printedPrompt = false;
			historyIndex = -1;
			break;
		// ^D → Quit
		case '\x04':
			// Don't quit if you are in the middle of typing something. This emulates Zsh.
			if (g_consoleCommand.length == 0) {
				putc('\n', stdout);
				g_cfg2.quit = true;
			}
			break;
		// \t → Code completion
		case '\t':
			error = terminal_codeCompletion(&g_consoleCommand, &badComplete, &tabs);
			if (error) {
				goto cleanup_l;
			}
			
			if (badComplete) {
				cursor = 0;
				badComplete = false;
			}
			else {
				tabs = 0;
			}
			
			while (cursor < g_consoleCommand.length) {
				// You should never see this.
				putc('#', stdout);
				cursor++;
			}
			while (cursor > 0) {
				putc('\b', stdout);
				putc(' ', stdout);
				putc('\b', stdout);
				--cursor;
			}
			while (cursor < g_consoleCommand.length) {
				putc(g_consoleCommand.value[cursor], stdout);
				cursor++;
			}
			fflush(stdout);
			break;
		case ' ':
		default:
			if (isprint(character)) {
				// Increase string size.
				if ((cursor + 1 >= g_consoleCommand.length) || insert) {
					g_consoleCommand.length++;
					error = string_realloc(&g_consoleCommand);
					if (error) {
						critical_error("Out of memory", "");
						error = ERR_OUTOFMEMORY;
						goto cleanup_l;
					}
					g_consoleCommand.value[g_consoleCommand.length] = '\0';
				}
				
				if (insert) {
					// Shift everything after the cursor to the right.
					for (int i = g_consoleCommand.length - 2; i >= cursor; --i) {
						g_consoleCommand.value[i + 1] = g_consoleCommand.value[i];
					}
				}
				
				// Add char to string.
				g_consoleCommand.value[cursor] = character;
				
				// Move cursor forward.
				if (g_consoleCommand.length > 0) {
					cursor++;
				}
				
				// Echo
				putc(character, stdout);
				if (insert) {
					// Note: Already incremented cursor.
					for (int i = cursor; i < g_consoleCommand.length; i++) {
						putc(g_consoleCommand.value[i], stdout);
					}
					// Backspace to original spot.
					for (int i = cursor; i < g_consoleCommand.length; i++) {
						putc('\b', stdout);
					}
				}
				fflush(stdout);
			}
		}
		
		if (character == '\t') {
			tabs++;
		}
		else {
			tabs = 0;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int terminal_getHistoryLine(string_t *line, int *index) {
	int error = ERR_CRITICAL;
	
	cfg2_var_t *v_historyLength = cfg2_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != cfg2_var_type_integer) {
		critical_error(CFG_HISTORY_LENGTH" is not an integer", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	int historyLength = v_historyLength->integer;
	
	if (*index >= historyLength) {
		*index = historyLength - 1;
	}
	if (*index < 0) {
		*index = 0;
	}
	
	while ((g_commandHistory[*index].length == 0) && (*index > 0)) {
		--*index;
	}
	
	string_copy(line, &g_commandHistory[*index]);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int terminal_addLineToHistory(const string_t *line) {
	int error = ERR_CRITICAL;
	
	int duplicate = -1;
	string_t tempString;
	
	cfg2_var_t *v_historyLength = cfg2_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != cfg2_var_type_integer) {
		critical_error(CFG_HISTORY_LENGTH" is not an integer", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	int historyLength = v_historyLength->integer;
	
	for (int i = 0; i < historyLength; i++) {
		if (!strcmp(line->value, g_commandHistory[i].value)) {
			duplicate = i;
			break;
		}
	}
	
	if (duplicate < 0) {
		string_free(&g_commandHistory[historyLength - 1]);
		for (int i = historyLength - 1; i > 0; --i) {
			g_commandHistory[i] = g_commandHistory[i - 1];
		}
	
		g_commandHistory[0].value = NULL;
		g_commandHistory[0].length = 0;
		g_commandHistory[0].memsize = 0;
		string_copy(&g_commandHistory[0], line);
	}
	else {
		tempString.value = g_commandHistory[duplicate].value;
		tempString.length = g_commandHistory[duplicate].length;
		tempString.memsize = g_commandHistory[duplicate].memsize;
		
		for (int i = duplicate; i > 0; --i) {
			g_commandHistory[i] = g_commandHistory[i - 1];
		}
		
		g_commandHistory[0].value = tempString.value;
		g_commandHistory[0].length = tempString.length;
		g_commandHistory[0].memsize = tempString.memsize;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

// @TODO: Actually use this function. I think I can do it now.
/* terminal_logCommandFrequency
Record the frequency that a variable is used as the destination.
line:   The line that was executed.
*/
void terminal_logCommandFrequency(const string_t *line) {
	
	char *command;
	char *spaceIndex;
	int length;
	bool normalize = false;
	
	command = line->value;
	spaceIndex = strchr(line->value, ' ');
	if (spaceIndex == NULL) {
		length = line->length;
	}
	else {
		length = spaceIndex - command;
	}
	
	// Increment the frequency
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		// if (length == strlen(g_cfg2.vars[i].name)) {
		if (!strncmp(command, g_cfg2.vars[i].name, length)) {
			g_cfg2.vars[i].frequency++;
			if (g_cfg2.vars[i].frequency > UINT_MAX / 2) {
				normalize = true;
			}
			break;
		}
		// }
	}
	
	// Prevent overflowing or saturating the frequency counters.
	if (normalize) {
		for (int i = 0; i < g_cfg2.vars_length; i++) {
			g_cfg2.vars[i].frequency /= 2;
		}
	}
}

int terminal_initConsole(void) {
	int error = ERR_CRITICAL;
	
	string_init(&g_consoleCommand);
	string_init(&g_commandComplete);
	
	cfg2_var_t *v_historyLength = cfg2_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != cfg2_var_type_integer) {
		critical_error(CFG_HISTORY_LENGTH" is not an integer", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_commandHistory = malloc(v_historyLength->integer * sizeof(string_t));
	if (g_commandHistory == NULL) {
		critical_error("Out of memory", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	for (int i = 0; i < v_historyLength->integer; i++) {
		error = string_init(&g_commandHistory[i]);
		if (error) {
			critical_error("Out of memory", "");
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

void terminal_quitConsole(void) {
	cfg2_var_t *v_historyLength = cfg2_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != cfg2_var_type_integer) {
		critical_error(CFG_HISTORY_LENGTH" is not an integer", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (g_commandHistory != NULL) {
		for (int i = 0; i < v_historyLength->integer; i++) {
			string_free(&g_commandHistory[i]);
		}
	}
	insane_free(g_commandHistory);
	
	cleanup_l:
	
	string_free(&g_commandComplete);
	string_free(&g_consoleCommand);
	
	return;
}
