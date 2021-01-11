
#include "cfg.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "file.h"
#include "log.h"
#include "insane.h"

#ifdef LINUX
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#endif

void cfg_logCommandFrequency(const string_t *line);

cfg_t g_cfg;
string_t g_consoleCommand;
string_t *g_commandHistory;
string_t g_commandComplete;

// This MUST be in ASCII alphabetical order.
cfg_commandList_t g_commandList = {
	.commands = {
		"+",
		"-",
		"=",
		"add",
		"clear",
		"cmds",
		"create",
		"delete",
		"exec",
		"exit",
		"help",
		"if",
		"ifdef",
		"ifndef",
		"print",
		"quit",
		"read",
		"set",
		"sub",
		"vars"
	},
	.arguments_min = {
		2,
		2,
		2,
		2,
		1,
		0,
		1,
		1,
		1,
		0,
		0,
		2,
		2,
		2,
		1,
		0,
		1,
		2,
		2,
		0
	},
	.arguments_max = {
		2,
		2,
		2,
		2,
		1,
		0,
		2,
		1,
		1,
		0,
		0,
		-1,
		-1,
		-1,
		-1,
		0,
		1,
		2,
		2,
		0
	},
	.length = CFG_NUM_COMMANDS
};

/* Vars */
/* ==== */

cfg_var_t *cfg_addVarNone(const char *name) {

	g_cfg.vars_length++;
	g_cfg.vars = realloc(g_cfg.vars, (g_cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (g_cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	g_cfg.vars[g_cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (g_cfg.vars[g_cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(g_cfg.vars[g_cfg.vars_length - 1].name, name);
	
	g_cfg.vars[g_cfg.vars_length - 1].string.value = NULL;
	g_cfg.vars[g_cfg.vars_length - 1].string.length = 0;
	g_cfg.vars[g_cfg.vars_length - 1].string.memsize = -1;
	g_cfg.vars[g_cfg.vars_length - 1].vector = 0;
	g_cfg.vars[g_cfg.vars_length - 1].integer = 0;
	
	g_cfg.vars[g_cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	g_cfg.vars[g_cfg.vars_length - 1].type = none;
	g_cfg.vars[g_cfg.vars_length - 1].handle = NULL;
	
	return &g_cfg.vars[g_cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarVector(const char *name, const vec_t value) {

	g_cfg.vars_length++;
	g_cfg.vars = realloc(g_cfg.vars, (g_cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (g_cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	g_cfg.vars[g_cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (g_cfg.vars[g_cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(g_cfg.vars[g_cfg.vars_length - 1].name, name);
	
	g_cfg.vars[g_cfg.vars_length - 1].string.value = NULL;
	g_cfg.vars[g_cfg.vars_length - 1].string.length = 0;
	g_cfg.vars[g_cfg.vars_length - 1].string.memsize = -1;
	g_cfg.vars[g_cfg.vars_length - 1].vector = value;
	g_cfg.vars[g_cfg.vars_length - 1].integer = 0;
	
	g_cfg.vars[g_cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	g_cfg.vars[g_cfg.vars_length - 1].type = vector;
	g_cfg.vars[g_cfg.vars_length - 1].handle = NULL;
	
	return &g_cfg.vars[g_cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarInt(const char *name, const int value) {
	
	g_cfg.vars_length++;
	g_cfg.vars = realloc(g_cfg.vars, (g_cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (g_cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	g_cfg.vars[g_cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (g_cfg.vars[g_cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(g_cfg.vars[g_cfg.vars_length - 1].name, name);
	
	g_cfg.vars[g_cfg.vars_length - 1].string.value = NULL;
	g_cfg.vars[g_cfg.vars_length - 1].string.length = 0;
	g_cfg.vars[g_cfg.vars_length - 1].string.memsize = -1;
	g_cfg.vars[g_cfg.vars_length - 1].vector = 0;
	g_cfg.vars[g_cfg.vars_length - 1].integer = value;
	
	g_cfg.vars[g_cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	g_cfg.vars[g_cfg.vars_length - 1].type = integer;
	g_cfg.vars[g_cfg.vars_length - 1].handle = NULL;
	
	return &g_cfg.vars[g_cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarString(const char *name, const char *value) {

	g_cfg.vars_length++;
	g_cfg.vars = realloc(g_cfg.vars, (g_cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (g_cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	g_cfg.vars[g_cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (g_cfg.vars[g_cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(g_cfg.vars[g_cfg.vars_length - 1].name, name);

	// g_cfg.vars[g_cfg.vars_length - 1].string = malloc((strlen(value) + 1) * sizeof(char));
	// if (g_cfg.vars[g_cfg.vars_length - 1].string == NULL) {
	// 	log_critical_error(__func__, "Out of memory");
	// 	return NULL;
	// }
	// strcpy(g_cfg.vars[g_cfg.vars_length - 1].string, value);
	
	g_cfg.vars[g_cfg.vars_length - 1].string.value = NULL;
	g_cfg.vars[g_cfg.vars_length - 1].string.length = 0;
	g_cfg.vars[g_cfg.vars_length - 1].string.memsize = -1;
	string_copy_c(&g_cfg.vars[g_cfg.vars_length - 1].string, value);
	
	g_cfg.vars[g_cfg.vars_length - 1].vector = 0;
	g_cfg.vars[g_cfg.vars_length - 1].integer = 0;
	
	g_cfg.vars[g_cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	g_cfg.vars[g_cfg.vars_length - 1].type = string;
	g_cfg.vars[g_cfg.vars_length - 1].handle = NULL;
	
	return &g_cfg.vars[g_cfg.vars_length - 1];
}

// Has side-effect of calling handle.
int cfg_setVarVector(cfg_var_t *var, const vec_t value) {
	int error = ERR_CRITICAL;
	
	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_WRITE)) {
		var->vector = value;
		
		// Run update function if it exists.
		if (var->handle != NULL) {
			error = var->handle(var);
			if (error) {
				goto cleanup_l;
			}
		}
		
		error = ERR_OK;
		goto cleanup_l;
	}

	error = ERR_GENERIC;
	cleanup_l:
	return error;
}

// Has side-effect of calling handle.
int cfg_setVarInt(cfg_var_t *var, const int value) {
	int error = ERR_CRITICAL;
	
	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_WRITE)) {
		var->integer = value;
		
		// Run update function if it exists.
		if (var->handle != NULL) {
			error = var->handle(var);
			if (error) {
				goto cleanup_l;
			}
		}
		
		error = ERR_OK;
		goto cleanup_l;
	}

	error = ERR_GENERIC;
	cleanup_l:
	return error;
}

// Has side-effect of calling handle.
int cfg_setVarString(cfg_var_t *var, const char *value) {
	int error = ERR_CRITICAL;

	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_WRITE)) {
		// var->string = realloc(var->string, (strlen(value) + 1) * sizeof(char));
		// if (var->string == NULL) {
		// 	log_critical_error(__func__, "Out of memory");
		// 	return ERR_OUTOFMEMORY;
		// }
		// strcpy(var->string, value);
		string_copy_c(&var->string, value);
		
		// Run update function if it exists.
		if (var->handle != NULL) {
			error = var->handle(var);
			if (error) {
				goto cleanup_l;
			}
		}
		
		error = ERR_OK;
		goto cleanup_l;
	}
	
	error = ERR_GENERIC;
	cleanup_l:
	return error;
}

int cfg_getVarVector(cfg_var_t *var, vec_t *value) {
	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_READ)) {
		*value = var->vector;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

int cfg_getVarInt(cfg_var_t *var, int *value) {
	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_READ)) {
		*value = var->integer;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

/* This returns the original, not a copy. */
int cfg_getVarString(cfg_var_t *var, string_t **value) {

	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_READ)) {
		*value = &var->string;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

cfg_var_t *cfg_findVar(const char *name) {
	for (int i = 0; i < g_cfg.vars_length; i++) {
		if (!strcmp(name, g_cfg.vars[i].name)) {
			return &g_cfg.vars[i];
		}
	}
	
	return NULL;
}

int cfg_deleteVar(cfg_var_t *var) {
	int index;

	if (var->permissions & CFG_VAR_FLAG_PROTECTED) {
		return ERR_GENERIC;
	}

	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_DELETE)) {
		index = var - g_cfg.vars;
		
		insane_free(var->name);
		string_free(&var->string);
		var->handle = NULL;
		
		--g_cfg.vars_length;
		for (int i = index; i < g_cfg.vars_length; i++) {
			g_cfg.vars[i] = g_cfg.vars[i+1];
		}
		return ERR_OK;
	}
	return ERR_GENERIC;
}

int cfg_initVars(const cfg_var_init_t *initCfgList) {

	if (initCfgList == NULL) {
		return ERR_GENERIC;
	}

	g_cfg.quit = false;
	g_cfg.lock = false;

	/* Find length so that we can call malloc once. */
	for (g_cfg.vars_length = 0; initCfgList[g_cfg.vars_length].name != NULL; g_cfg.vars_length++);
	
	g_cfg.vars = malloc(g_cfg.vars_length * sizeof(cfg_var_t));
	if (g_cfg.vars == NULL) {
		return ERR_OUTOFMEMORY;
	}
	
	for (int i = 0; i < g_cfg.vars_length; i++) {
		g_cfg.vars[i].name = malloc((strlen(initCfgList[i].name) + 1) * sizeof(char));
		if (g_cfg.vars[i].name == NULL) {
			return ERR_OUTOFMEMORY;
		}
		strcpy(g_cfg.vars[i].name, initCfgList[i].name);
		
		g_cfg.vars[i].vector = initCfgList[i].vector;
		g_cfg.vars[i].integer = initCfgList[i].integer;
		
		if (string_init(&g_cfg.vars[i].string)) {
			return ERR_OUTOFMEMORY;
		}
		if (initCfgList[i].string != NULL) {
			// cfg.vars[i].string = malloc((strlen(initCfgList[i].string) + 1) * sizeof(char));
			// if (cfg.vars[i].string == NULL) {
			// 	return ERR_OUTOFMEMORY;
			// }
			// strcpy(cfg.vars[i].string, initCfgList[i].string);
			string_copy_c(&g_cfg.vars[i].string, initCfgList[i].string);
		}
		else {
			// cfg.vars[i].string.v = NULL;
		}
		
		g_cfg.vars[i].type = initCfgList[i].type;
		g_cfg.vars[i].permissions = initCfgList[i].permissions | CFG_VAR_FLAG_PROTECTED;
		
		g_cfg.vars[i].handle = initCfgList[i].handle;
		if (g_cfg.vars[i].handle != NULL) {
			if (g_cfg.vars[i].handle(&g_cfg.vars[i])) {
				return ERR_CRITICAL;
			}
		}
	}
	
	return ERR_OK;
}

int cfg_printVar(cfg_var_t *var, const char *tag) {

	if (!g_cfg.lock || (var->permissions & CFG_VAR_FLAG_READ)) {
		switch (var->type) {
			case none:
				printf(COLOR_CYAN"%s: "COLOR_BLUE"[%s]"COLOR_NORMAL"\n", tag, var->name);
				break;
			case vector:
				printf(COLOR_CYAN"%s: "COLOR_BLUE"[%s]"COLOR_CYAN" %f"COLOR_NORMAL"\n", tag, var->name, var->vector);
				break;
			case integer:
				printf(COLOR_CYAN"%s: "COLOR_BLUE"[%s]"COLOR_CYAN" %i"COLOR_NORMAL"\n", tag, var->name, var->integer);
				break;
			case string:
				printf(COLOR_CYAN"%s: "COLOR_BLUE"[%s]"COLOR_CYAN" \"%s\""COLOR_NORMAL"\n", tag, var->name, var->string.value);
				break;
			default:
				log_error(__func__, "Can't happen");
				return ERR_GENERIC;
		}
		return ERR_OK;
	}
	return ERR_GENERIC;
}

/* Config */
/* ====== */

void cfg_free(void) {
	
	for (int i = 0; i < g_cfg.vars_length; i++) {
		insane_free(g_cfg.vars[i].name);
		string_free(&g_cfg.vars[i].string);
	}
	insane_free(g_cfg.vars);
	g_cfg.vars_length = 0;
}

/* Scripting */
/* ========= */

int cfg_handle_maxRecursion(cfg_var_t *var) {
	if (var->integer <= 0) {
		var->integer = 1;
		error("%s is negative or zero. Setting to %i", var->name, var->integer);
	}
	return ERR_OK;
}

/* Execute a single line of the config file. */
int cfg_execString(const string_t *line, const char *tag, const int recursionDepth) {

	int tempIndex;
	string_t command;
	int argc;
	string_t arg0;
	string_t arg1;
	int error = 0;
	cfg_var_t *tempCfgVar0 = NULL;
	cfg_var_t *tempCfgVar1 = NULL;
	cfg_var_type_t type;
	char *tempString = NULL;
	
	string_init(&command);
	string_init(&arg0);
	string_init(&arg1);
	
	/* Impose recursion limit. */
	
	tempCfgVar0 = cfg_findVar(CFG_MAX_RECURSION);
	if (tempCfgVar0 == NULL) {
		critical_error("Variable \""CFG_MAX_RECURSION"\" not defined.", "");
		error = ERR_CRITICAL;
		goto end_l;
	}
	if (tempCfgVar0->type != integer) {
		critical_error("Variable \""CFG_MAX_RECURSION"\" is not an integer.", "");
		error = ERR_CRITICAL;
		goto end_l;
	}
	if (recursionDepth >= tempCfgVar0->integer) {
		error("Reached limit of %i recursions.", tempCfgVar0->integer);
		error = ERR_GENERIC;
		goto end_l;
	}
	
	
	tempIndex = string_index_of(line, 0, ' ');
	string_substring(&command, line, 0, tempIndex);
	
	argc = string_count_char(line, ' ');

	if (!strcmp(command.value, "")) {
		// Do nothing. Also, DO NOT REMOVE.
	}
	/* Usage: ifdef variable command */
	else if (!strcmp(command.value, "ifdef")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires a variable and a command.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		/* Get the var to test. */
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		if (cfg_findVar(arg0.value) == NULL) {
			/* Variable doesn't exist. That's fine. Just skip the rest of the command. */
			error = ERR_OK;
			goto end_l;
		}

		/* The var exists. Execute the rest of the command. */
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		error = cfg_execString(&arg0, tag, recursionDepth + 1);
		if (error) {
			goto end_l;
		}
	}
	/* Usage: ifndef variable command */
	else if (!strcmp(command.value, "ifndef")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires a variable and a command.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		/* Get the var to test. */
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		if (cfg_findVar(arg0.value) != NULL) {
			/* Variable exists. Skip the rest of the command. */
			error = ERR_OK;
			goto end_l;
		}

		/* The var exists. Execute the rest of the command. */
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		error = cfg_execString(&arg0, tag, recursionDepth + 1);
		if (error) {
			goto end_l;
		}
	}
	/* Usage: set variable value */
	else if (!strcmp(command.value, "set")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		
		switch (tempCfgVar0->type) {
			case none:
				error("Cannot set variable \"%s\" since is of type \"none\".", tempCfgVar0->string.value);
				break;
			case string:
				cfg_setVarString(tempCfgVar0, arg0.value);
				break;
			case integer:
				cfg_setVarInt(tempCfgVar0, strtol(arg0.value, NULL, 10));
				break;
			case vector:
				cfg_setVarVector(tempCfgVar0, strtof(arg0.value, NULL));
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
		}
	}
	/* Usage: create [type] variable */
	else if (!strcmp(command.value, "create")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 1 or 2 arguments.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 1 or 2 arguments.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		if (argc == 1) {
			string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);
			
			tempCfgVar0 = cfg_findVar(arg0.value);
			if (tempCfgVar0 != NULL) {
				warning("Variable \"%s\" already exists", arg0.value);
				error = ERR_OK;
				goto end_l;
			}
			
			cfg_addVarString(arg0.value, "");
		}
		else {
			
			
			tempIndex = string_index_of(line, 0, ' ') + 1;
			string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
			
			if (!strcmp(arg0.value, "none")) {
				type = none;
			}
			else if (!strcmp(arg0.value, "vector")) {
				type = vector;
			}
			else if (!strcmp(arg0.value, "integer")) {
				type = integer;
			}
			else if (!strcmp(arg0.value, "string")) {
				type = string;
			}
			else {
				error("Unrecognized type \"%s\".", arg0.value);
				error = ERR_GENERIC;
				goto end_l;
			}
			
			string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
			
			tempCfgVar0 = cfg_findVar(arg0.value);
			if (tempCfgVar0 != NULL) {
				warning("Variable \"%s\" already exists", arg0.value);
				error = ERR_OK;
				goto end_l;
			}
			
			switch (type) {
			case none:
				cfg_addVarNone(arg0.value);
				break;
			case vector:
				cfg_addVarVector(arg0.value, 0);
				break;
			case integer:
				cfg_addVarInt(arg0.value, 0);
				break;
			case string:
				cfg_addVarString(arg0.value, "");
				break;
			default:
				critical_error("Can't happen.", "");
				error = ERR_CRITICAL;
				goto end_l;
			}
		}
	}
	/* Usage: delete variable */
	else if (!strcmp(command.value, "delete")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		if (argc > 1) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}

		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);

		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Variable \"%s\" already deleted", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		error = cfg_deleteVar(tempCfgVar0);
		if (error) {
			error("Unable to delete variable \"%s\"", tempCfgVar0->name);
			if (tempCfgVar0->permissions & CFG_VAR_FLAG_PROTECTED) {
				info("Oh ho ho! You're not deleting that, not even as administrator.", "");
			}
			error = ERR_GENERIC;
			goto end_l;
		}
	}
	/* Usage: print string */
	else if (!strcmp(command.value, "print")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires at least 1 argument.", command.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);
		
		printf(COLOR_CYAN"%s: "COLOR_NORMAL"%s\n", tag, arg0.value);
	}
	/* Usage: exec filename */
	else if (!strcmp(command.value, "exec")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires at least 1 argument.", command.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);
		
		error = cfg_execFile(arg0.value, recursionDepth + 1);
		if (error) {
			goto end_l;
		}
	}
	/* Usage: quit | exit */
	else if (!strcmp(command.value, "quit") || !strcmp(command.value, "exit")) {
		if (argc > 0) {
			error("Command \"%s\" has too many arguments. No arguments should be provided.", command.value);
			error = 1;
			goto end_l;
		}
		
		g_cfg.quit = true;
	}
	/* Usage: vars | variables */
	else if (!strcmp(command.value, "vars")) {
		if (argc > 0) {
			error("Command \"%s\" has too many arguments. No arguments should be provided.", command.value);
			error = 1;
			goto end_l;
		}
		
		const char treeMiddle[] = "├─ ";
		const char treeEnd[]    = "└─ ";
		char treeChars[]        = "── ";
		int j = 0;
		
		printf(COLOR_CYAN"%s:"COLOR_NORMAL"\n", tag);
		
		for (int i = 0; i < g_cfg.vars_length; i++) {
			if (!g_cfg.lock || (g_cfg.vars[i].permissions & CFG_VAR_FLAG_READ)) {
				if (i == g_cfg.vars_length - 1) {
					strcpy(treeChars, treeEnd);
				}
				else {
					strcpy(treeChars, treeMiddle);
				}
				j++;
			
				switch (g_cfg.vars[i].type) {
					case none:
						printf(COLOR_CYAN"%s"COLOR_BLUE"[%s]"COLOR_NORMAL"\n", treeChars, g_cfg.vars[i].name);
						break;
					case vector:
						printf(COLOR_CYAN"%s"COLOR_BLUE"[%s]"COLOR_CYAN" %f"COLOR_NORMAL"\n", treeChars, g_cfg.vars[i].name, g_cfg.vars[i].vector);
						break;
					case integer:
						printf(COLOR_CYAN"%s"COLOR_BLUE"[%s]"COLOR_CYAN" %i"COLOR_NORMAL"\n", treeChars, g_cfg.vars[i].name, g_cfg.vars[i].integer);
						break;
					case string:
						printf(COLOR_CYAN"%s"COLOR_BLUE"[%s]"COLOR_CYAN" \"%s\""COLOR_NORMAL"\n", treeChars, g_cfg.vars[i].name, g_cfg.vars[i].string.value);
						break;
					default:
						log_error(__func__, "Can't happen");
						error = 1;
						goto end_l;
				}
			}
			else {
				warning("Permission denied to read variable %s", g_cfg.vars[i].name);
			}
		}
	}
	/* Usage: cmds | commands */
	else if (!strcmp(command.value, "cmds")) {
		printf(COLOR_CYAN"%s:\n", tag);
		for (int i = 0; i < g_commandList.length; i++) {
			printf(COLOR_BLUE"%s"COLOR_NORMAL"\n", g_commandList.commands[i]);
		}
	}
	/* Usage: if variable command */
	else if (!strcmp(command.value, "if")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires a variable and a command.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		/* Get the var to test. */
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			error("Variable \"%s\" doesn't exist.", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		switch (tempCfgVar0->type) {
		case none:
			error = ERR_OK;
			goto end_l;
		case vector:
			if (tempCfgVar0->vector == 0.0) {
				error = ERR_OK;
				goto end_l;
			}
			break;
		case integer:
			if (tempCfgVar0->integer == 0) {
				error = ERR_OK;
				goto end_l;
			}
			break;
		case string:
			if (!strcmp(tempCfgVar0->string.value, "")) {
				error = ERR_OK;
				goto end_l;
			}
			break;
		default:
			critical_error("Can't happen. Type is %i", tempCfgVar0->type);
			error = ERR_CRITICAL;
			goto end_l;
		}
		
		/* The var exists. Execute the rest of the command. */
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		error = cfg_execString(&arg0, tag, recursionDepth + 1);
		if (error) {
			goto end_l;
		}
	}
	/* Usage: + destination source */
	else if (!strcmp(command.value, "+") || !strcmp(command.value, "add")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		
		tempCfgVar1 = cfg_findVar(arg0.value);
		if (tempCfgVar1 == NULL) {
			error("Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		switch (tempCfgVar0->type) {
			case none:
				error("Cannot add variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
				break;
			case string:
				// @TODO: Concatenate strings.
				error("Cannot add variable \"%s\" since is of type \"string\".", tempCfgVar0->name);
				break;
			case integer:
			case vector:
				switch (tempCfgVar1->type) {
				case none:
				case string:
					error("Cannot add non-numeric variables (\"%s\", \"%s\")", tempCfgVar0->name, tempCfgVar1->name);
					break;
				case vector:
				case integer:
					cfg_setVarInt(tempCfgVar0, tempCfgVar0->integer + tempCfgVar1->integer);
					break;
				default:
					log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar1->type, tempCfgVar1->name);
					error = ERR_CRITICAL;
					goto end_l;
				}
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
		}
	}
	/* Usage: - destination source */
	else if (!strcmp(command.value, "-") || !strcmp(command.value, "sub")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		
		tempCfgVar1 = cfg_findVar(arg0.value);
		if (tempCfgVar1 == NULL) {
			error("Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		switch (tempCfgVar0->type) {
			case none:
				error("Cannot subtract variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
				break;
			case string:
				// @TODO: Concatenate strings.
				error("Cannot subtract variable \"%s\" since is of type \"string\".", tempCfgVar0->name);
				break;
			case integer:
			case vector:
				switch (tempCfgVar1->type) {
				case none:
				case string:
					error("Cannot subtract non-numeric variables (\"%s\", \"%s\")", tempCfgVar0->name, tempCfgVar1->name);
					break;
				case vector:
				case integer:
					cfg_setVarInt(tempCfgVar0, tempCfgVar0->integer - tempCfgVar1->integer);
					break;
				default:
					log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar1->type, tempCfgVar1->name);
					error = ERR_CRITICAL;
					goto end_l;
				}
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
		}
	}
	/* Usage: = destination source */
	else if (!strcmp(command.value, "=") || !strcmp(command.value, "copy")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 2 arguments.", command.value);
			error = 1;
			goto end_l;
		}
		
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		
		tempCfgVar1 = cfg_findVar(arg0.value);
		if (tempCfgVar1 == NULL) {
			error("Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		switch (tempCfgVar0->type) {
		case none:
			error("Cannot copy variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
			break;
		case string:
			switch(tempCfgVar1->type) {
			case none:
				error("Cannot copy variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
				break;
			case string:
				cfg_setVarString(tempCfgVar0, tempCfgVar1->string.value);
				break;
			case integer:
				tempString = malloc((3 * sizeof(int) + 1) * sizeof(char));
				sprintf(tempString, "%i", tempCfgVar1->integer);
				cfg_setVarString(tempCfgVar0, tempString);
				insane_free(tempString);
				break;
			case vector:
				tempString = malloc((3 * sizeof(int) + 1) * sizeof(char));
				sprintf(tempString, "%f", tempCfgVar1->vector);
				cfg_setVarString(tempCfgVar0, tempString);
				insane_free(tempString);
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
			}
			break;
		case integer:
			switch(tempCfgVar1->type) {
			case none:
				error("Cannot copy variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
				break;
			case string:
				cfg_setVarInt(tempCfgVar0, strtol(tempCfgVar1->string.value, NULL, 10));
				break;
			case integer:
				cfg_setVarInt(tempCfgVar0, tempCfgVar1->integer);
				break;
			case vector:
				cfg_setVarInt(tempCfgVar0, round(tempCfgVar1->vector));
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
			}
			break;
		case vector:
			switch(tempCfgVar1->type) {
			case none:
				error("Cannot copy variable \"%s\" since is of type \"none\".", tempCfgVar0->name);
				break;
			case string:
				cfg_setVarVector(tempCfgVar0, strtod(tempCfgVar1->string.value, NULL));
				break;
			case integer:
				cfg_setVarVector(tempCfgVar0, tempCfgVar1->integer);
				break;
			case vector:
				cfg_setVarVector(tempCfgVar0, tempCfgVar1->vector);
				break;
			default:
				log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
				error = ERR_CRITICAL;
				goto end_l;
			}
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
			error = ERR_CRITICAL;
			goto end_l;
		}
	}
	/* Usage: read variable */
	else if (!strcmp(command.value, "read")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		if (argc > 1) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}

		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);

		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			error("Variable \"%s\" does not exist", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		cfg_printVar(tempCfgVar0, tag);
	}
	/* Usage: help */
	else if (!strcmp(command.value, "help")) {
		printf(COLOR_CYAN"%s: "COLOR_YELLOW"Try entering \"cmds\" or \"vars\"."COLOR_NORMAL"\n", ENGINE_MAN_NAME);
		printf(COLOR_CYAN"%s: "COLOR_YELLOW"If you are trying to escape (like me), type \"quit\", \"exit\", or <CTRL>+D."COLOR_NORMAL"\n", ENGINE_MAN_NAME);
		printf(COLOR_CYAN"%s: "COLOR_YELLOW"Hopefully the developer got around to writing a manual for this."COLOR_NORMAL"\n", ENGINE_MAN_NAME);
	}
	/* Usage: delete variable */
	else if (!strcmp(command.value, "clear")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		if (argc > 1) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 1 argument.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}

		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);

		tempCfgVar0 = cfg_findVar(arg0.value);
		if (tempCfgVar0 == NULL) {
			log_error(__func__, "Variable \"%s\" already deleted", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		switch (tempCfgVar0->type) {
		case none:
			error("Variable \"%s\" is already cleared since is of type \"none\".", tempCfgVar0->name);
			break;
		case string:
			cfg_setVarString(tempCfgVar0, "");
			break;
		case integer:
			cfg_setVarInt(tempCfgVar0, 0);
			break;
		case vector:
			cfg_setVarVector(tempCfgVar0, 0.0);
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", tempCfgVar0->type, tempCfgVar0->name);
			error = ERR_CRITICAL;
			goto end_l;
		}
	}
	else {
		/* Usage: variable */
		/* Before returning an error, see if a variable exists. */
		if (argc == 0) {
			tempCfgVar0 = cfg_findVar(command.value);
			/* Doubtful */
			if (tempCfgVar0 != NULL) {
				cfg_printVar(tempCfgVar0, tag);
				error = 0;
				goto end_l;
			}
		}
	
		log_error(__func__, "Unknown command or variable \"%s\"", line->value);
		error = 1;
		goto end_l;
	}
	
	if ((recursionDepth == 0) && !strcmp(tag, "console")) {
		error = cfg_addLineToHistory(line);
		if (error) {
			goto end_l;
		}
		cfg_logCommandFrequency(line);
	}
	
	error = ERR_OK;
	
	end_l:
	
	string_free(&command);
	string_free(&arg0);
	string_free(&arg1);
	
	return error;
}

int cfg_execFile(const char *filepath, const int recursionDepth) {
	
	FILE *file;
	string_t line;
	int error = 0;
	cfg_var_t *v_quiet;
	int quiet = 0;
	
	file = fopen(filepath, "r");
	if (file == NULL) {
		log_error(__func__, "Could not open file %s", filepath);
		return 1;
	}
	
	string_init(&line);
	
	v_quiet = cfg_findVar(CFG_RUN_QUIET);
	if (v_quiet == NULL) {
		warning(CFG_RUN_QUIET" is undefined.", "");
	}
	else if (v_quiet->type != integer) {
		warning(CFG_RUN_QUIET" is not an integer.", "");
	}
	else {
		quiet = v_quiet->integer;
	}
	
	if (!(quiet & 1)) {
		log_info(__func__, "Executing \"%s\"", filepath);
	}
	
	for (int linenumber = 1;; linenumber++) {
		
		error = file_getLine(&line, '\n', file);
		if (error) {
			if (error < 0) {
				error = ERR_OK;
			}
			break;
		}
		
		/* Remove comments and unnecessary whitespace. */
		string_removeLineComments(&line, "#");
		string_removeWhitespace(&line, "melt");
		/* Convert remaining whitespace to spaces */
		for (int i = 0; i < line.length; i++) {
			if (isspace(line.value[i])) {
				line.value[i] = ' ';
			}
		}
		
		/* Discard empty lines. */
		if (line.length == 0) {
			continue;
		}
		
		error = cfg_execString(&line, (quiet & 2) ? "" : filepath, recursionDepth);
		if (error) {
			if (recursionDepth == 0) {
				log_error(__func__, "Error on line %i", linenumber);
			}
			else {
				error("Error on line %i, recursion level %i", linenumber, recursionDepth);
			}
			break;
		}
	}
	
	string_free(&line);
	fclose(file);
	
	return error;
}

/* Terminal */
/* ======== */

int cfg_terminalInit(void) {
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

int cfg_handle_updateCommandHistoryLength(cfg_var_t *var) {
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

// @TODO: This is horrible! Rewrite!! FIXME!!!
static int cfg_fragmentCompletion(string_t *fragment, bool *badComplete, int *tabs, int priority) {
	int error = ERR_CRITICAL;

	bool commandPotentials[CFG_NUM_COMMANDS];
	int commandPotentialsCount = 0;
	bool variablePotentials[g_cfg.vars_length];
	int variablePotentialsCount = 0;
	int tabIndex = 0;
	int totalPotentialsCount;
	const int maxPotentials = 10;
	int potentialsLoopCounter = 0;
	
	// Find matching commands.
	for (int i = 0; i < CFG_NUM_COMMANDS; i++) {
		commandPotentials[i] = !strncmp(fragment->value, g_commandList.commands[i], fragment->length);
		if (commandPotentials[i]) {
			commandPotentialsCount++;
		}
	}
	
	// Find matching variables.
	for (int i = 0; i < g_cfg.vars_length; i++) {
		variablePotentials[i] = !strncmp(fragment->value, g_cfg.vars[i].name, fragment->length);
		if (variablePotentials[i]) {
			variablePotentialsCount++;
		}
	}
	
	totalPotentialsCount = commandPotentialsCount + variablePotentialsCount;
	if (totalPotentialsCount > maxPotentials) {
		totalPotentialsCount = maxPotentials;
	}
	
	if (totalPotentialsCount > 1) {
		*badComplete = 1;
		putc('\n', stdout);
	}
	
	if (*tabs >= totalPotentialsCount) {
		*tabs = 0;
	}
	
	for (int i = 0, v, c; (i < g_commandList.length + g_cfg.vars_length) && (potentialsLoopCounter < totalPotentialsCount); i++) {
		if (priority) {
			// Variables first
			c = (i >= g_cfg.vars_length) ? i - g_cfg.vars_length : -1;
			v = (i >= g_cfg.vars_length) ? -1 : i;
		}
		else {
			// Commands first
			c = (i >= g_commandList.length) ? -1 : i;
			v = (i >= g_commandList.length) ? i - g_commandList.length : -1;
		}
		
		if ((c >= 0) && commandPotentials[c]) {
			// One correction
			if ((totalPotentialsCount == 1) && (commandPotentialsCount == 1)) {
				error = string_copy_c(fragment, g_commandList.commands[c]);
				if (error) {
					goto cleanup_l;
				}
				
				if (g_commandList.arguments_min[c] > 0) {
					error = string_append_char(fragment, ' ');
					if (error) {
						goto cleanup_l;
					}
				}
				break;
			}
			// Multiple corrections
			else {
				if (tabIndex == *tabs) {
				
					string_copy_c(&g_commandComplete, g_commandList.commands[c]);
					if (g_commandList.arguments_min[c] > 0) {
						error = string_append_char(&g_commandComplete, ' ');
						if (error) {
							goto cleanup_l;
						}
					}
					
					printf(COLOR_BLACK B_COLOR_WHITE"%s"COLOR_NORMAL"\n", g_commandList.commands[c]);
				}
				else {
					puts(g_commandList.commands[c]);
				}
				tabIndex++;
			}
			potentialsLoopCounter++;
		}
		if ((v >= 0) && variablePotentials[v]) {
			// One correction
			if ((totalPotentialsCount == 1) && (variablePotentialsCount == 1)) {
				error = string_copy_c(fragment, g_cfg.vars[v].name);
				if (error) {
					goto cleanup_l;
				}
				break;
			}
			// Multiple corrections
			else {
				if (tabIndex == *tabs) {
					string_copy_c(&g_commandComplete, g_cfg.vars[v].name);
					printf(COLOR_BLACK B_COLOR_WHITE"%s"COLOR_NORMAL"\n", g_cfg.vars[v].name);
				}
				else {
					puts(g_cfg.vars[v].name);
				}
				tabIndex++;
			}
			potentialsLoopCounter++;
		}
	}
	
	if (totalPotentialsCount > 1) {
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

static int cfg_codeCompletion(string_t *line, bool *badComplete, int *tabs) {
	int error = ERR_CRITICAL;
	char *start, *space;
	int length;
	string_t fragment;
	// Consider commands first, then variables.
	int priority = 0;
	
	error = string_init(&fragment);
	if (error) {
		goto cleanup_l;
	}
	
	start = line->value;
	space = NULL;
	
	while (1) {
		space = strchr(start, ' ');
		if (space == NULL) {
			break;
		}
		start = space + 1;
		priority++;
	}
	
	length = line->value + line->length - start;
	
	string_copy_length_c(&fragment, start, length);
	
	// Do command complete.
	if (space == NULL) {
		error = cfg_fragmentCompletion(&fragment, badComplete, tabs, priority > 0);
		if (error) {
			goto cleanup_l;
		}
	}
	
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

static int cfg_enterCompletion(string_t *line) {
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

int cfg_runTerminalCommand(void) {
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
				cfg_getHistoryLine(&g_consoleCommand, &historyIndex);
				
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
				cfg_getHistoryLine(&g_consoleCommand, &historyIndex);
				
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
				error = cfg_enterCompletion(&g_consoleCommand);
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
			error = cfg_execString(&g_consoleCommand, "console", 0);
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
				g_cfg.quit = true;
			}
			break;
		// \t → Code completion
		case '\t':
			error = cfg_codeCompletion(&g_consoleCommand, &badComplete, &tabs);
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

int cfg_getHistoryLine(string_t *line, int *index) {
	int error = ERR_CRITICAL;
	
	cfg_var_t *v_historyLength = cfg_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != integer) {
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

int cfg_addLineToHistory(const string_t *line) {
	int error = ERR_CRITICAL;
	
	int duplicate = -1;
	string_t tempString;
	
	cfg_var_t *v_historyLength = cfg_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != integer) {
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

void cfg_logCommandFrequency(const string_t *line) {
	
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
	for (int i = 0; i < g_commandList.length; i++) {
		if (length == strlen(g_commandList.commands[i])) {
			if (!strncmp(command, g_commandList.commands[i], length)) {
				g_commandList.frequency[i]++;
				if (g_commandList.frequency[i] > INT_MAX / 2) {
					normalize = true;
				}
				break;
			}
		}
	}
	
	if (normalize) {
		for (int i = 0; i < g_commandList.length; i++) {
			g_commandList.frequency[i] /= 2;
		}
	}
}

int cfg_initConsole(void) {
	int error = ERR_CRITICAL;
	
	string_init(&g_consoleCommand);
	string_init(&g_commandComplete);
	
	cfg_var_t *v_historyLength = cfg_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != integer) {
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

void cfg_quitConsole(void) {
	cfg_var_t *v_historyLength = cfg_findVar(CFG_HISTORY_LENGTH);
	if (v_historyLength == NULL) {
		critical_error(CFG_HISTORY_LENGTH" is not defined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	if (v_historyLength->type != integer) {
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
