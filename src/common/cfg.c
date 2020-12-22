
#include "cfg.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "log.h"
#include "insane.h"

cfg_t cfg;


/* Vars */
/* ==== */

cfg_var_t *cfg_addVarNone(const char *name) {

	cfg.vars_length++;
	cfg.vars = realloc(cfg.vars, (cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	cfg.vars[cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (cfg.vars[cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(cfg.vars[cfg.vars_length - 1].name, name);
	
	cfg.vars[cfg.vars_length - 1].string.value = NULL;
	string_normalize(&cfg.vars[cfg.vars_length - 1].string);
	cfg.vars[cfg.vars_length - 1].vector = 0;
	cfg.vars[cfg.vars_length - 1].integer = 0;
	
	cfg.vars[cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	cfg.vars[cfg.vars_length - 1].type = none;
	
	return &cfg.vars[cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarVector(const char *name, const vec_t value) {

	cfg.vars_length++;
	cfg.vars = realloc(cfg.vars, (cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	cfg.vars[cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (cfg.vars[cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(cfg.vars[cfg.vars_length - 1].name, name);
	
	cfg.vars[cfg.vars_length - 1].string.value = NULL;
	string_normalize(&cfg.vars[cfg.vars_length - 1].string);
	cfg.vars[cfg.vars_length - 1].vector = value;
	cfg.vars[cfg.vars_length - 1].integer = 0;
	
	cfg.vars[cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	cfg.vars[cfg.vars_length - 1].type = vector;
	
	return &cfg.vars[cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarInt(const char *name, const int value) {
	
	cfg.vars_length++;
	cfg.vars = realloc(cfg.vars, (cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	cfg.vars[cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (cfg.vars[cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(cfg.vars[cfg.vars_length - 1].name, name);
	
	cfg.vars[cfg.vars_length - 1].string.value = NULL;
	string_normalize(&cfg.vars[cfg.vars_length - 1].string);
	cfg.vars[cfg.vars_length - 1].vector = value;
	cfg.vars[cfg.vars_length - 1].integer = 0;
	
	cfg.vars[cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	cfg.vars[cfg.vars_length - 1].type = integer;
	
	return &cfg.vars[cfg.vars_length - 1];
}

cfg_var_t *cfg_addVarString(const char *name, const char *value) {

	cfg.vars_length++;
	cfg.vars = realloc(cfg.vars, (cfg.vars_length + 1) * sizeof(cfg_var_t));
	if (cfg.vars == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	
	cfg.vars[cfg.vars_length - 1].name = malloc((strlen(name) + 1) * sizeof(char));
	if (cfg.vars[cfg.vars_length - 1].name == NULL) {
		log_critical_error(__func__, "Out of memory");
		return NULL;
	}
	strcpy(cfg.vars[cfg.vars_length - 1].name, name);

	// cfg.vars[cfg.vars_length - 1].string = malloc((strlen(value) + 1) * sizeof(char));
	// if (cfg.vars[cfg.vars_length - 1].string == NULL) {
	// 	log_critical_error(__func__, "Out of memory");
	// 	return NULL;
	// }
	// strcpy(cfg.vars[cfg.vars_length - 1].string, value);
	string_copy_c(&cfg.vars[cfg.vars_length - 1].string, value);
	
	cfg.vars[cfg.vars_length - 1].vector = 0;
	cfg.vars[cfg.vars_length - 1].integer = 0;
	
	cfg.vars[cfg.vars_length - 1].permissions = CFG_VAR_PERMISSION_ALL;
	cfg.vars[cfg.vars_length - 1].type = string;
	
	return &cfg.vars[cfg.vars_length - 1];
}

int cfg_setVarVector(cfg_var_t *var, const vec_t value) {
	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_WRITE)) {
		var->vector = value;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

int cfg_setVarInt(cfg_var_t *var, const int value) {
	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_WRITE)) {
		var->integer = value;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

int cfg_setVarString(cfg_var_t *var, const char *value) {

	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_WRITE)) {
		// var->string = realloc(var->string, (strlen(value) + 1) * sizeof(char));
		// if (var->string == NULL) {
		// 	log_critical_error(__func__, "Out of memory");
		// 	return ERR_OUTOFMEMORY;
		// }
		// strcpy(var->string, value);
		string_copy_c(&var->string, value);
		return ERR_OK;
	}

	return ERR_GENERIC;
}

int cfg_getVarVector(cfg_var_t *var, vec_t *value) {
	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_READ)) {
		*value = var->vector;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

int cfg_getVarInt(cfg_var_t *var, int *value) {
	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_READ)) {
		*value = var->integer;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

/* This returns the original, not a copy. */
int cfg_getVarString(cfg_var_t *var, string_t **value) {

	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_READ)) {
		*value = &var->string;
		return ERR_OK;
	}

	return ERR_GENERIC;
}

cfg_var_t *cfg_findVar(const char *name) {
	for (int i = 0; i < cfg.vars_length; i++) {
		if (!strcmp(name, cfg.vars[i].name)) {
			return &cfg.vars[i];
		}
	}
	
	return NULL;
}

int cfg_deleteVar(cfg_var_t *var) {
	int index;

	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_DELETE)) {
		index = var - cfg.vars;
		
		insane_free(var->name);
		string_free(&var->string);
		
		--cfg.vars_length;
		for (int i = index; i < cfg.vars_length; i++) {
			cfg.vars[i] = cfg.vars[i+1];
		}
		return ERR_OK;
	}
	return ERR_GENERIC;
}

int cfg_initVars(const cfg_var_init_t *initCfgList) {

	if (initCfgList == NULL) {
		return ERR_GENERIC;
	}

	cfg.lock = false;

	/* Find length so that we can call malloc once. */
	for (cfg.vars_length = 0; initCfgList[cfg.vars_length].name != NULL; cfg.vars_length++);
	
	cfg.vars = malloc(cfg.vars_length * sizeof(cfg_var_t));
	if (cfg.vars == NULL) {
		return ERR_OUTOFMEMORY;
	}
	
	for (int i = 0; i < cfg.vars_length; i++) {
		cfg.vars[i].name = malloc((strlen(initCfgList[i].name) + 1) * sizeof(char));
		if (cfg.vars[i].name == NULL) {
			return ERR_OUTOFMEMORY;
		}
		strcpy(cfg.vars[i].name, initCfgList[i].name);
		
		cfg.vars[i].vector = initCfgList[i].vector;
		cfg.vars[i].integer = initCfgList[i].integer;
		
		if (string_init(&cfg.vars[i].string)) {
			return ERR_OUTOFMEMORY;
		}
		if (initCfgList[i].string != NULL) {
			// cfg.vars[i].string = malloc((strlen(initCfgList[i].string) + 1) * sizeof(char));
			// if (cfg.vars[i].string == NULL) {
			// 	return ERR_OUTOFMEMORY;
			// }
			// strcpy(cfg.vars[i].string, initCfgList[i].string);
			string_copy_c(&cfg.vars[i].string, initCfgList[i].string);
		}
		else {
			// cfg.vars[i].string.v = NULL;
		}
		
		cfg.vars[i].type = initCfgList[i].type;
		cfg.vars[i].permissions = initCfgList[i].permissions;
	}
	
	return ERR_OK;
}

int cfg_printVar(cfg_var_t *var, const char *tag) {
	string_t value;

	if (!cfg.lock || (var->permissions & CFG_VAR_PERMISSION_READ)) {
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
				printf(COLOR_CYAN"%s: "COLOR_BLUE"[%s]"COLOR_CYAN" \"%s\""COLOR_NORMAL"\n", tag, var->name, var->string);
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
	
	for (int i = 0; i < cfg.vars_length; i++) {
		insane_free(cfg.vars[i].name);
		string_free(&cfg.vars[i].string);
	}
	insane_free(cfg.vars);
	cfg.vars_length = 0;
}

/* Scripting */
/* ========= */

/* Execute a single line of the config file. */
int cfg_execString(const string_t *line, const char *tag) {

	int tempIndex;
	string_t command;
	int argc;
	string_t arg0;
	string_t arg1;
	int error = 0;
	cfg_var_t *tempCfgVar = NULL;
	
	string_init(&command);
	string_init(&arg0);
	string_init(&arg1);
	
	tempIndex = string_index_of(line, 0, ' ');
	string_substring(&command, line, 0, tempIndex);
	
	argc = string_count_char(line, ' ');

	if (!strcmp(command.value, "ifdef")) {
		if (argc < 3) {
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
		cfg_execString(&arg0, tag);
	}
	else if (!strcmp(command.value, "ifndef")) {
		if (argc < 3) {
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
		cfg_execString(&arg0, tag);
	}
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
		
		tempCfgVar = cfg_findVar(arg0.value);
		if (tempCfgVar == NULL) {
			log_error(__func__, "Unable to set variable \"%s\"", arg0.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 1, ' ') + 1, -1);
		
		cfg_setVarString(tempCfgVar, arg0.value);
	}
	else if (!strcmp(command.value, "create")) {
		if (argc < 2) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires 2 arguments.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		if (argc > 2) {
			log_error(__func__, "Command \"%s\" has too many arguments. Requires 2 arguments.", command.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		tempIndex = string_index_of(line, 0, ' ') + 1;
		string_substring(&arg0, line, tempIndex, string_index_of(line, 1, ' ') - tempIndex);
		
		tempCfgVar = cfg_findVar(arg0.value);
		if (tempCfgVar != NULL) {
			log_error(__func__, "Variable \"%s\" already exists", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		
		string_substring(&arg1, line, string_index_of(line, 1, ' ') + 1, -1);
		
		/* @TODO: Create other types besides strings. */
		cfg_addVarString(arg0.value, arg1.value);
	}
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

		tempCfgVar = cfg_findVar(arg0.value);
		if (tempCfgVar == NULL) {
			log_error(__func__, "Variable \"%s\" already deleted", arg0.value);
			error = ERR_GENERIC;
			goto end_l;
		}
		cfg_deleteVar(tempCfgVar);
	}
	else if (!strcmp(command.value, "print")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires at least 1 argument.", command.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);
		
		printf(COLOR_CYAN"%s: "COLOR_NORMAL"%s\n", tag, arg0.value);
	}
	else if (!strcmp(command.value, "exec")) {
		if (argc < 1) {
			log_error(__func__, "Command \"%s\" has too few arguments. Requires at least 1 argument.", command.value);
			error = 1;
			goto end_l;
		}
		
		string_substring(&arg0, line, string_index_of(line, 0, ' ') + 1, -1);
		
		cfg_execFile(arg0.value);
	}
	else {
		/* Before returning an error, see if a variable exists. */
		if (argc == 0) {
			tempCfgVar = cfg_findVar(command.value);
			/* Doubtful */
			if (tempCfgVar != NULL) {
				cfg_printVar(tempCfgVar, tag);
				error = 0;
				goto end_l;
			}
		}
	
		log_error(__func__, "Unknown command or variable \"%s\"", line->value);
		error = 1;
		goto end_l;
	}
	
	error = ERR_OK;
	
	end_l:
	
	string_free(&command);
	string_free(&arg0);
	string_free(&arg1);
	
	return error;
}

int cfg_execFile(const char *filepath) {
	
	FILE *file;
	string_t line;
	int error = 0;
	
	file = fopen(filepath, "r");
	if (file == NULL) {
		log_error(__func__, "Could not open file %s", filepath);
		return 1;
	}
	
	log_info(__func__, "Executing \"%s\"", filepath);
	
	string_init(&line);
	
	for (int linenumber = 1;; linenumber++) {
		
		error = file_getLine(&line, '\n', file);
		if (error) {
			break;
		}
		
		/* Remove comments and unnecessary whitespace. */
		string_removeLineComments(&line, '#');
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
		
		error = cfg_execString(&line, filepath);
		if (error) {
			log_error(__func__, "Syntax error on line %i", linenumber);
			// break;
		}
	}
	
	error_label:
	
	string_free(&line);
	fclose(file);
	
	return error;
}
