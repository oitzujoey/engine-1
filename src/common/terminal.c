
#include "terminal.h"
#include "common.h"
#include "log.h"
#include "cfg2.h"
#include "str2.h"

#ifdef LINUX
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#endif

char *g_consoleCommand;
size_t g_consoleCommand_length = 0;
char **g_commandHistory;
size_t g_commandHistoryLength = 0;
char *g_commandComplete;

int terminal_getHistoryLine(char *line, int *index);
int terminal_addLineToHistory(const char *line);
void terminal_logCommandFrequency(const char *line);

/* Terminal callbacks */
/* ================== */

int terminal_callback_updateCommandHistoryLength(cfg2_var_t *var, const char *command) {
	int error = ERR_CRITICAL;
	
	static int lastLength = CFG_HISTORY_LENGTH_DEFAULT;
	
	g_commandHistoryLength = var->integer;
	
	if (g_commandHistoryLength <= 0) {
		var->integer = 1;
		g_commandHistoryLength = var->integer;
		warning(CFG_HISTORY_LENGTH" is not a whole number. Setting to %i.", var->integer);
	}
	
	if (g_commandHistoryLength != lastLength) {
	
		if (g_commandHistoryLength < lastLength) {
			// Need to free.
			for (int i = g_commandHistoryLength; i < lastLength; i++) {
				insane_free(g_commandHistory[i]);
			}
		}
	
		g_commandHistory = realloc(g_commandHistory, g_commandHistoryLength * sizeof(char *));
		if (g_commandHistory == NULL) {
			critical_error("Out of memory", "");
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		if (g_commandHistoryLength > lastLength) {
			// Need to allocate.
			for (int i = lastLength; i < g_commandHistoryLength; i++) {
				g_commandHistory[i] = NULL;
			}
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	lastLength = g_commandHistoryLength;
	
	return error;
}

/* Terminal functions */
/* ================== */

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

// This is for the function below.
static int terminal_intCompare(const void *left, const void *right) {
	return g_cfg2.vars[*((int *) left)].frequency < g_cfg2.vars[*((int *) right)].frequency;
}

/* terminal_fragmentCompletion
Completes a command fragment. badComplete is set if multiple options have been
found. Tabs allow selection when multiple options are available.
fragment:       The partial command that is to be completed.
badComplete:    True if there are multiple completions.
tabs:           The number of times the user has pressed <tab> so far.
*/
static int terminal_fragmentCompletion(char *fragment, bool *badComplete, int *tabs) {
	int error = ERR_CRITICAL;

	const int maxPotentials = 10;
	int variablePotentials[g_cfg2.vars_length];
	int variablePotentialsCount = 0;
	int tabIndex = 0;
	
	// Find matching completions.
	
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		if (!strncmp(fragment, g_cfg2.vars[i].name, strlen(fragment))) {
			variablePotentials[variablePotentialsCount] = i;
			variablePotentialsCount++;
		}
	}
	
	// Sort completions by frequency used.
	
	qsort(variablePotentials, variablePotentialsCount, sizeof(int), terminal_intCompare);
	
	// Limit number of completions that are shown.
	
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
	
	// Complete or display completions.
	
	for (int i = 0; i < variablePotentialsCount; i++) {
		
		// One correction
		if (variablePotentialsCount == 1) {
			error = str2_copyMalloc(fragment, g_cfg2.vars[variablePotentials[i]].name);
			if (error) {
				goto cleanup_l;
			}
			break;
		}
		// Multiple corrections
		else {
			if (tabIndex == *tabs) {
				str2_copyMalloc(g_commandComplete, g_cfg2.vars[variablePotentials[i]].name);
				printf(COLOR_BLACK B_COLOR_WHITE"%s"COLOR_NORMAL"\n", g_cfg2.vars[variablePotentials[i]].name);
			}
			else {
				puts(g_cfg2.vars[variablePotentials[i]].name);
			}
			tabIndex++;
		}
	}
	
	// Print prompt, or do single completion.
	
	if (variablePotentialsCount > 1) {
		printf("> ");
		fflush(stdout);
	}
	else {
		str2_copyMalloc(g_commandComplete, "");
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
static int terminal_codeCompletion(char *line, bool *badComplete, int *tabs) {
	int error = ERR_CRITICAL;
	char *start, *space;
	int length;
	char *fragment = NULL;
	
	start = line;
	space = NULL;
	
	// Find the last variable on the line. The variable is probably incomplete.
	while (1) {
		space = strchr(start, ' ');
		if (space == NULL) {
			break;
		}
		start = space + 1;
	}
	
	length = line + strlen(line) - start;
	
	// Copy the last variable into "fragment".
	// string_copy_length_c(&fragment, start, length);
	error = str2_copyLengthMalloc(fragment, start, length);
	if (error) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Do command complete on the fragment.
	if (space == NULL) {
		error = terminal_fragmentCompletion(fragment, badComplete, tabs);
		if (error) {
			goto cleanup_l;
		}
	}
	
	// Stick the completed variable back on the end of the line.
	start[0] = '\0';
	// error = string_normalize(line);
	// if (error) {
	// 	goto cleanup_l;
	// }
	// error = string_concatenate(line, &fragment);
	error = str2_concatenateMalloc(line, fragment);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	insane_free(fragment);
	
	return error;
}

/* terminal_enterCompletion
Complete the fragment and replace it with the user-selected variable.
*/
static int terminal_enterCompletion(char *line) {
	int error = ERR_CRITICAL;
	char *start, *space;
	
	start = line;
	space = NULL;
	
	while (1) {
		space = strchr(start, ' ');
		if (space == NULL) {
			break;
		}
		start = space + 1;
	}
	
	start[0] = '\0';
	error = str2_concatenateMalloc(line, g_commandComplete);
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
	static cfg2_admin_t adminLevel = cfg2_admin_administrator;
	cfg2_admin_t tempAdminLevel;
	
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
				terminal_getHistoryLine(g_consoleCommand, &historyIndex);
				
				while (cursor < g_consoleCommand_length) {
					putc('#', stdout);
					cursor++;
				}
				while (cursor > 0) {
					putc('\b', stdout);
					putc(' ', stdout);
					putc('\b', stdout);
					--cursor;
				}
				while (cursor < g_consoleCommand_length) {
					putc(g_consoleCommand[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			// Cursor down (Down arrow) → Down in history
			case 'B':
				--historyIndex;
				terminal_getHistoryLine(g_consoleCommand, &historyIndex);
				
				while (cursor < g_consoleCommand_length) {
					putc('#', stdout);
					cursor++;
				}
				while (cursor > 0) {
					putc('\b', stdout);
					putc(' ', stdout);
					putc('\b', stdout);
					--cursor;
				}
				while (cursor < g_consoleCommand_length) {
					putc(g_consoleCommand[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			// Cursor forward (Right arrow)
			case 'C':
				if (cursor < g_consoleCommand_length) {
					putc(g_consoleCommand[cursor], stdout);
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
			if ((tabs > 0) && (strlen(g_commandComplete) != 0)) {
				tabs = 0;
				error = terminal_enterCompletion(g_consoleCommand);
				if (error) {
					goto cleanup_l;
				}
				// cursor = 0;
				while (cursor < g_consoleCommand_length) {
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
				while (cursor < g_consoleCommand_length) {
					putc(g_consoleCommand[cursor], stdout);
					cursor++;
				}
				fflush(stdout);
				break;
			}
		
			putc('\n', stdout);
			
			// // Execute string as administrator.
			tempAdminLevel = g_cfg2.adminLevel;
			// The terminal has its own adminLevel.
			g_cfg2.adminLevel = adminLevel;
			g_cfg2.recursionDepth = 0;
			error = cfg2_execString(g_consoleCommand, "console");
			// adminLevel may have been modified by a command.
			adminLevel = g_cfg2.adminLevel;
			g_cfg2.adminLevel = tempAdminLevel;
			
			if (error == ERR_OUTOFMEMORY) {
				critical_error("Out of memory", "");
				goto cleanup_l;
			}
			if (error == ERR_CRITICAL) {
				goto cleanup_l;
			}
			
			error = terminal_addLineToHistory(g_consoleCommand);
			if (error) {
				goto cleanup_l;
			}
			terminal_logCommandFrequency(g_consoleCommand);
			
			printedPrompt = false;
			historyIndex = -1;
			cursor = 0;
			g_consoleCommand_length = 0;
			break;
		// Backspace
		case '\x7F':
			if (cursor > 0) {
				for (int i = cursor; i < g_consoleCommand_length; i++) {
					g_consoleCommand[i - 1] = g_consoleCommand[i];
				}
				g_consoleCommand[g_consoleCommand_length - 1] = ' ';
				
				tempInt = cursor;
				--cursor;
				putc('\b', stdout);
				
				while (cursor < g_consoleCommand_length) {
					putc(g_consoleCommand[cursor], stdout);
					cursor++;
				}
				
				--tempInt;
				while (cursor > tempInt) {
					--cursor;
					putc('\b', stdout);
				}
				
				fflush(stdout);
				
				--g_consoleCommand_length;
				g_consoleCommand[g_consoleCommand_length] = '\0';
			}
			break;
		// ESC
		case '\x1B':
			escape = true;
			break;
		// ^C → Cease conjuring
		case '\x03':
			putc('\n', stdout);
			g_consoleCommand_length = 0;
			g_consoleCommand[0] = '\0';
			cursor = 0;
			printedPrompt = false;
			historyIndex = -1;
			break;
		// ^D → Quit
		case '\x04':
			// Don't quit if you are in the middle of typing something. This emulates Zsh.
			if (g_consoleCommand_length == 0) {
				putc('\n', stdout);
				g_cfg2.quit = true;
			}
			break;
		// \t → Code completion
		case '\t':
			error = terminal_codeCompletion(g_consoleCommand, &badComplete, &tabs);
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
			
			while (cursor < g_consoleCommand_length) {
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
			while (cursor < g_consoleCommand_length) {
				putc(g_consoleCommand[cursor], stdout);
				cursor++;
			}
			fflush(stdout);
			break;
		case ' ':
		default:
			if (isprint(character)) {
				// Increase string size.
				if ((cursor + 1 >= g_consoleCommand_length) || insert) {
					g_consoleCommand_length++;
					error = str2_realloc(g_consoleCommand, g_consoleCommand_length);
					if (error) {
						critical_error("Out of memory", "");
						error = ERR_OUTOFMEMORY;
						goto cleanup_l;
					}
					g_consoleCommand[g_consoleCommand_length] = '\0';
				}
				
				if (insert) {
					// Shift everything after the cursor to the right.
					for (int i = g_consoleCommand_length - 2; i >= cursor; --i) {
						g_consoleCommand[i + 1] = g_consoleCommand[i];
					}
				}
				
				// Add char to string.
				g_consoleCommand[cursor] = character;
				
				// Move cursor forward.
				if (g_consoleCommand_length > 0) {
					cursor++;
				}
				
				// Echo
				putc(character, stdout);
				if (insert) {
					// Note: Already incremented cursor.
					for (int i = cursor; i < g_consoleCommand_length; i++) {
						putc(g_consoleCommand[i], stdout);
					}
					// Backspace to original spot.
					for (int i = cursor; i < g_consoleCommand_length; i++) {
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

int terminal_getHistoryLine(char *line, int *index) {
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
	
	while ((strlen(g_commandHistory[*index]) == 0) && (*index > 0)) {
		--*index;
	}
	
	str2_copyMalloc(line, g_commandHistory[*index]);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int terminal_addLineToHistory(const char *line) {
	int error = ERR_CRITICAL;
	
	int duplicate = -1;
	char *tempString;
	
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
		if (!strcmp(line, g_commandHistory[i])) {
			duplicate = i;
			break;
		}
	}
	
	if (duplicate < 0) {
		insane_free(g_commandHistory[historyLength - 1]);
		for (int i = historyLength - 1; i > 0; --i) {
			g_commandHistory[i] = g_commandHistory[i - 1];
		}
	
		g_commandHistory[0] = NULL;
		str2_copy(g_commandHistory[0], line);
	}
	else {
		tempString = g_commandHistory[duplicate];
		
		for (int i = duplicate; i > 0; --i) {
			g_commandHistory[i] = g_commandHistory[i - 1];
		}
		
		g_commandHistory[0] = tempString;
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
void terminal_logCommandFrequency(const char *line) {
	
	const char *command;
	char *spaceIndex;
	int length;
	bool normalize = false;
	
	command = line;
	spaceIndex = strchr(line, ' ');
	if (spaceIndex == NULL) {
		length = strlen(line);
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
	
	g_consoleCommand = NULL;
	g_commandComplete = NULL;
	
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
	
	g_commandHistory = malloc(v_historyLength->integer * sizeof(char *));
	if (g_commandHistory == NULL) {
		critical_error("Out of memory", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	for (int i = 0; i < v_historyLength->integer; i++) {
		g_commandHistory[i] = NULL;
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
			insane_free(g_commandHistory[i]);
		}
	}
	insane_free(g_commandHistory);
	
	cleanup_l:
	
	insane_free(g_commandComplete);
	insane_free(g_consoleCommand);
	
	return;
}
