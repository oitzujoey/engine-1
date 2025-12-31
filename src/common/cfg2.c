#include "cfg2.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "common.h"
#include "log.h"
#include "str2.h"
#include "file.h"
#include "../physfs-3.0.2/src/physfs.h"
#include "memory.h"

#ifdef CLIENT
#include "../client/input.h"
#endif

/*
It might be better to use Lua for all my configuration.

I will still have an init structure. This will create Lua variables and set function handles.
There will be three Lua states: supervisor, administrator, and game. An admin can gain supervisor privleges at any time. A warning will be shown that tells the user about the dangers of supervisor privleges.
Any Lua state can set any config variables it wishes, however, those variables will only be copied to the main config state if the current Lua state has the proper permissions.

Variable types (CFG in C)
	none
	integer
	vector (float/double)
	string (char *)
	function?

Init (supervisor)
	All variables can be set.
	Variables and functions are copied to the config list.
Supervisor
	All variables can be set.
	All changed variables are copied to the config list.
Administrator
	All variables can be set.
	All changed non-supervisor variables are copied to the config list.
Game
	All variables can be set.
	All changed non-supervisor and non-administrator variables are copied to the config list.
*/

/*
Lua scripting system:

Lua will call functions to create, set, 

Console
	There will be no true console commands. The format will be:
	<string> <strings>
	strings:
		<string>
		<strings> <strings>
	string:
		"<identifier> <...> <identifier>"
		<identifier>
	identifier:
		Any string of characters other than '"'. If a '"' does exist in the string, it must be escaped with a '\'.
	...:
		<identifier>
		<...> <...>
	The first string will be the name of a variable. If the variable does not exist, then the command will return with an error. If the variable does exist, then an assignment will be attempted. Everything after the first variable will be assigned to that variable. Additional actions may occur if a callback has been passed to that variable.
	Using this implementation, we can get an effect that is very similar to standard config scripting. "set" is not a command but a variable that calls a function when a string is assigned to it. 
	
	Scripting example:
		workspace .
		ifdef server exec server.cfg
		ifdef client exec client.cfg
		net_timeout 5000
		max_recursion_depth 100



Lua variable table
	* value
	int index
C variable structure
	float vector;
	int integer;
	char *string;
	int type;
	int (*callback)(cfg_var_t *var, lua_State *luaState);
	int permissions;

table variable, int status = cfg2_createVariable(string name, * value [, int type])
	Create a variable named "name" with a value "value" and type "typeof(value)". Returns error if variable already exists.
int status = cfg2_deleteVariable(table variable)
	Delete the given variable. Returns error if variable does not exist or is restricted.
table variable, int status = cfg2_findVariable(string name)
	Find the variable with the given name. Returns error if variable does not exist or is restricted.
int status = cfg2_setVariable(table variable, * value)
	Set an already existing variable to "value". Returns an error if the variable does not exist, is restricted, or is of the wrong type.
table variable, int status = cfg2_getVariable(table variable)
	Update Lua's value of the variable. Returns an error if the variable doesn't exist or is restricted.

int status = cfg2_setCallback(table variable, function callback)
	Sets the callback of "variable" to "callback". It will be run every time this variable is set. If 0 is given as the callback, the callback will be disabled for that variable. Returns error if the variable does not exist or is restricted.
function callback, int status = cfg2_getCallback(table variable)
	Gets the callback that is attached to the variable. Returns error if the variable does not exist or is restricted.
*/

cfg2_t g_cfg2;

/* cfg2 commands */
/* ============= */

int cfg2_callback_set(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	// At this point, var->command has been set to the command string. Nothing else has been modified.
	
	// // First, let's get rid of var->name in the string.
	// char *start = strchr(var->command, ' ') + 1;
	// int offset = start - var->command;
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// // Delete var->name + ' '.
	// for (int i = 0; i < strlen(var->command) + 1 - offset; i++) {
	// 	var->command[i] = var->command[i + offset];
	// }
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = arg1 + strlen(arg1) + 1;
	// if (*arg2 = '\0') {
	
	// }
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Check var1 permissions.
	if (g_cfg2.adminLevel < var1->permissionWrite) {
		warning("Permissions not high enough to set variable %s.", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		warning("Cannot set variable \"%s\" since it is of type \"none\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	case cfg2_var_type_vector:
#ifdef DOUBLE_VEC
		var1->vector = strtod(arg2, NULL);
#else
		var1->vector = strtof(arg2, NULL);
#endif
		break;
	case cfg2_var_type_integer:
		var1->integer = strtol(arg2, NULL, 10);
		break;
	case cfg2_var_type_string:
		var1->string = realloc(var1->string, (strlen(arg2) + 1) * sizeof(char));
		if (var1->string == NULL) {
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		strcpy(var1->string, arg2);
		break;
	default:
		critical_error("Bad variable type %i.", var1->type);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Run callback.
	if (var1->callback != NULL) {
		error = var1->callback(var1, arg2, luaState);
		if (error) {
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	MEMORY_FREE(&commandCopy);
	
	return error;
}

int cfg2_callback_ifdef(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	arg2 = arg1 + strlen(arg1) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	// Convert ',' to ';'.
	str2_replaceChar(arg2, ',', ';');
	
	error = cfg2_execString(arg2, luaState, NULL);
	// No variable pointers are safe beyond this point.
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_exec(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	// At this point, var->command has been set to the command string. Nothing else has been modified.
	
	// // First, let's get rid of var->name in the string.
	// char *start = strchr(var->command, ' ') + 1;
	// int offset = start - var->command;
	char *arg1 = NULL, *arg2 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// // Delete var->name + ' '.
	// for (int i = 0; i < strlen(var->command) + 1 - offset; i++) {
	// 	var->command[i] = var->command[i + offset];
	// }
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Execute file.
	
	g_cfg2.recursionDepth++;
	cfg2_execFile(arg1, luaState);
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_create(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	cfg2_var_t *var2 = NULL;
	cfg2_var_type_t type2;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg3 = strtok(NULL, " ");
	if (arg3 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	// arg3 = arg2 + strlen(arg2) + 1 + (command - commandCopy);
	
	if (!strcmp(arg1, "none")) {
		type2 = cfg2_var_type_none;
	}
	else if (!strcmp(arg1, "command")) {
		type2 = cfg2_var_type_none;
	}
	else if (!strcmp(arg1, "integer")) {
		type2 = cfg2_var_type_integer;
	}
	else if (!strcmp(arg1, "vector")) {
		type2 = cfg2_var_type_vector;
	}
	else if (!strcmp(arg1, "string")) {
		type2 = cfg2_var_type_string;
	}
	else {
		warning("Bad type \"%s\".", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	var2 = cfg2_findVar(arg2);
	if (var2 != NULL) {
		warning("Variable \"%s\" already exists.", arg2);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	info("Creating %s \"%s\" with permission level %i.", arg1, arg2, g_cfg2.adminLevelDisguise);
	cfg2_createVariable(var2, arg2, type2, g_cfg2.adminLevelDisguise);
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_quit(cfg2_var_t *var, const char *command, lua_State *luaState) {

	// Prevent execution on creation.
	static int created = 0;
	// 2, because there are two commands: quit and exit.
	if (created < 2) {
		created++;
		return ERR_OK;
	}
	
	g_cfg2.quit = true;
	return ERR_OK;
}

int cfg2_callback_vars(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;

	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	const char treeMiddle[] = "├─ ";
	const char treeEnd[]    = "└─ ";
	char treeChars[]        = "── ";
	int j = 0;
	
	// printf(COLOR_CYAN"%s:"COLOR_NORMAL"\n", NULL);
	
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		if (g_cfg2.adminLevel >= g_cfg2.vars[i].permissionRead) {
			if (i == g_cfg2.vars_length - 1) {
				strcpy(treeChars, treeEnd);
			}
			else {
				strcpy(treeChars, treeMiddle);
			}
			j++;
			
			switch (g_cfg2.vars[i].type) {
				case cfg2_var_type_none:
					printf(COLOR_CYAN"%s"COLOR_BLUE"n[%s]"COLOR_NORMAL"\n", treeChars, g_cfg2.vars[i].name);
					break;
				case cfg2_var_type_vector:
					printf(COLOR_CYAN"%s"COLOR_BLUE"v[%s]"COLOR_CYAN" %f"COLOR_NORMAL"\n", treeChars, g_cfg2.vars[i].name, g_cfg2.vars[i].vector);
					break;
				case cfg2_var_type_integer:
					printf(COLOR_CYAN"%s"COLOR_BLUE"i[%s]"COLOR_CYAN" %i"COLOR_NORMAL"\n", treeChars, g_cfg2.vars[i].name, g_cfg2.vars[i].integer);
					break;
				case cfg2_var_type_string:
					printf(COLOR_CYAN"%s"COLOR_BLUE"s[%s]"COLOR_CYAN" \"%s\""COLOR_NORMAL"\n", treeChars, g_cfg2.vars[i].name, g_cfg2.vars[i].string);
					break;
				default:
					log_error(__func__, "Can't happen");
					error = 1;
					goto cleanup_l;
			}
		}
		else {
			error("Insufficient permission to read variable \"%s\".", g_cfg2.vars[i].name);
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int cfg2_callback_copy(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static int created = 0;
	if (created < 2) {
		created++;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	cfg2_var_t *var1 = NULL, *var2 = NULL;
	char *tempString = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg3 = strtok(NULL, " ");
	if (arg3 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	// arg3 = arg2 + strlen(arg2) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	var2 = cfg2_findVar(arg2);
	if (var2 == NULL) {
		warning("Variable \"%s\" does not exist.", arg2);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var1->permissionWrite) {
		error("Insufficient permission to write variable \"%s\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var2->permissionRead) {
		error("Insufficient permission to read variable \"%s\".", var2->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		error("Cannot copy variable \"%s\" since is of type \"none\".", var1->name);
		break;
	case cfg2_var_type_string:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot copy variable \"%s\" since is of type \"none\".", var1->name);
			break;
		case cfg2_var_type_string:
			// cfg_setVarString(var1, var2->string.value);
			var1->string = realloc(var1->string, (strlen(var2->string) + 1) * sizeof(char));
			if (var1->string == NULL) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			strcpy(var1->string, var2->string);
			break;
		case cfg2_var_type_integer:
			tempString = malloc((3 * sizeof(int) + 1) * sizeof(char));
			sprintf(tempString, "%i", var2->integer);
			var1->string = realloc(var1->string, (strlen(tempString) + 1) * sizeof(char));
			if (var1->string == NULL) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			strcpy(var1->string, tempString);
			MEMORY_FREE(&tempString);
			break;
		case cfg2_var_type_vector:
			tempString = malloc((3 * sizeof(int) + 1) * sizeof(char));
			sprintf(tempString, "%f", var2->vector);
			var1->string = realloc(var1->string, (strlen(tempString) + 1) * sizeof(char));
			if (var1->string == NULL) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			strcpy(var1->string, tempString);
			MEMORY_FREE(&tempString);
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_integer:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot copy variable \"%s\" since is of type \"none\".", var1->name);
			break;
		case cfg2_var_type_string:
			var1->integer = strtol(var2->string, NULL, 10);
			break;
		case cfg2_var_type_integer:
			var1->integer = var2->integer;
			break;
		case cfg2_var_type_vector:
			warning("Rounding \"%s\" to nearest integer before addition.", var2->name);
			var1->integer = round(var2->vector);
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_vector:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot copy variable \"%s\" since is of type \"none\".", var1->name);
			break;
		case cfg2_var_type_string:
			var1->vector = strtod(var2->string, NULL);
			break;
		case cfg2_var_type_integer:
			var1->vector = var2->integer;
			break;
		case cfg2_var_type_vector:
			var1->vector = var2->vector;
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	default:
		log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_adminLevel(cfg2_var_t *var, const char *command, lua_State *luaState) {
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	printf(COLOR_CYAN"You are a level %i admin."COLOR_NORMAL"\n", g_cfg2.adminLevel);
	
	return ERR_OK;
}

int cfg2_callback_su(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	if (g_cfg2.adminLevel < cfg2_admin_administrator) {
		warning("Cannot elevate privileges from game.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (strcmp(command, "")) {
		g_cfg2.adminLevel = strtol(command, NULL, 10);
		if (g_cfg2.adminLevel < cfg2_admin_game) {
			g_cfg2.adminLevel = cfg2_admin_game;
		}
		if (g_cfg2.adminLevel > cfg2_admin_supervisor) {
			g_cfg2.adminLevel = cfg2_admin_supervisor;
		}
	}
	else if (g_cfg2.adminLevel == cfg2_admin_administrator) {
		g_cfg2.adminLevel = cfg2_admin_supervisor;
	}
	else if (g_cfg2.adminLevel == cfg2_admin_supervisor) {
		g_cfg2.adminLevel = cfg2_admin_administrator;
	}
	
	if (g_cfg2.adminLevel == cfg2_admin_supervisor) {
		warning("You are now in supervisor mode. All variables are unlocked.", "");
	}
	
	g_cfg2.adminLevelDisguise = g_cfg2.adminLevel;
	
	printf(COLOR_CYAN"You are a level %i admin."COLOR_NORMAL"\n", g_cfg2.adminLevel);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int cfg2_callback_suDisguise(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	if (g_cfg2.adminLevel < cfg2_admin_administrator) {
		warning("Cannot elevate privileges from game.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (strcmp(command, "")) {
		g_cfg2.adminLevelDisguise = strtol(command, NULL, 10);
		if (g_cfg2.adminLevelDisguise < cfg2_admin_game) {
			g_cfg2.adminLevelDisguise = cfg2_admin_game;
		}
		if (g_cfg2.adminLevelDisguise > cfg2_admin_supervisor) {
			g_cfg2.adminLevelDisguise = cfg2_admin_supervisor;
		}
	}
	else if (g_cfg2.adminLevelDisguise == cfg2_admin_administrator) {
		g_cfg2.adminLevelDisguise = cfg2_admin_supervisor;
	}
	else if (g_cfg2.adminLevelDisguise == cfg2_admin_supervisor) {
		g_cfg2.adminLevelDisguise = cfg2_admin_administrator;
	}
	
	printf(COLOR_CYAN"You are a pretend level %i admin and a real level %i admin."COLOR_NORMAL"\n", g_cfg2.adminLevelDisguise, g_cfg2.adminLevel);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int cfg2_callback_add(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static int created = 0;
	if (created < 2) {
		created++;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	cfg2_var_t *var1 = NULL, *var2 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg3 = strtok(NULL, " ");
	if (arg3 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	// arg3 = arg2 + strlen(arg2) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	var2 = cfg2_findVar(arg2);
	if (var2 == NULL) {
		warning("Variable \"%s\" does not exist.", arg2);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var1->permissionWrite) {
		error("Insufficient permission to write variable \"%s\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var2->permissionRead) {
		error("Insufficient permission to read variable \"%s\".", var2->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		error("Cannot add variable \"%s\" since is of type \"none\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	case cfg2_var_type_string:
		error("Cannot add variable \"%s\" since is of type \"string\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	case cfg2_var_type_integer:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot add variable \"%s\" since is of type \"none\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
		case cfg2_var_type_string:
			error("Cannot add variable \"%s\" since is of type \"string\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
		case cfg2_var_type_integer:
			var1->integer += var2->integer;
			break;
		case cfg2_var_type_vector:
			warning("Rounding \"%s\" to nearest integer before addition.", var2->name);
			var1->integer += round(var2->vector);
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_vector:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot add variable \"%s\" since is of type \"none\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
			break;
		case cfg2_var_type_string:
			error("Cannot add variable \"%s\" since is of type \"string\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
			break;
		case cfg2_var_type_integer:
			var1->vector += var2->integer;
			break;
		case cfg2_var_type_vector:
			var1->vector += var2->vector;
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	default:
		log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_sub(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static int created = 0;
	if (created < 2) {
		created++;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;
	cfg2_var_t *var1 = NULL, *var2 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg3 = strtok(NULL, " ");
	if (arg3 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	// arg3 = arg2 + strlen(arg2) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	var2 = cfg2_findVar(arg2);
	if (var2 == NULL) {
		warning("Variable \"%s\" does not exist.", arg2);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var1->permissionWrite) {
		error("Insufficient permission to write variable \"%s\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (g_cfg2.adminLevel < var2->permissionRead) {
		error("Insufficient permission to read variable \"%s\".", var2->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		error("Cannot subtract variable \"%s\" since is of type \"none\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	case cfg2_var_type_string:
		error("Cannot subtract variable \"%s\" since is of type \"string\".", var1->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	case cfg2_var_type_integer:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot subtract variable \"%s\" since is of type \"none\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
		case cfg2_var_type_string:
			error("Cannot subtract variable \"%s\" since is of type \"string\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
		case cfg2_var_type_integer:
			var1->integer -= var2->integer;
			break;
		case cfg2_var_type_vector:
			warning("Rounding \"%s\" to nearest integer before addition.", var2->name);
			var1->integer -= round(var2->vector);
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_vector:
		switch(var2->type) {
		case cfg2_var_type_none:
			error("Cannot subtract variable \"%s\" since is of type \"none\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
			break;
		case cfg2_var_type_string:
			error("Cannot subtract variable \"%s\" since is of type \"string\".", var2->name);
			error = ERR_GENERIC;
			goto cleanup_l;
			break;
		case cfg2_var_type_integer:
			var1->vector -= var2->integer;
			break;
		case cfg2_var_type_vector:
			var1->vector -= var2->vector;
			break;
		default:
			log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		break;
	default:
		log_critical_error(__func__, "Illegal type \"%i\" for variable \"%s\".", var1->type, var1->name);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_if(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	arg2 = arg1 + strlen(arg1) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		error = ERR_OK;
		goto cleanup_l;
	case cfg2_var_type_vector:
		if (var1->vector == 0.0) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_integer:
		if (var1->integer == 0) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_string:
		if (!strcmp(var1->string, "")) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	default:
		critical_error("Can't happen. Type is %i", var1->type);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Convert ',' to ';'.
	str2_replaceChar(arg2, ',', ';');
	
	error = cfg2_execString(arg2, luaState, NULL);
	// No variable pointers are safe beyond this point.
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_ifn(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	arg2 = arg1 + strlen(arg1) + 1 + (command - commandCopy);
	
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	switch (var1->type) {
	case cfg2_var_type_none:
		error = ERR_OK;
		goto cleanup_l;
	case cfg2_var_type_vector:
		if (var1->vector != 0.0) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_integer:
		if (var1->integer != 0) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	case cfg2_var_type_string:
		if (strcmp(var1->string, "")) {
			error = ERR_OK;
			goto cleanup_l;
		}
		break;
	default:
		critical_error("Can't happen. Type is %i", var1->type);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// Convert ',' to ';'.
	str2_replaceChar(arg2, ',', ';');
	
	error = cfg2_execString(arg2, luaState, NULL);
	// No variable pointers are safe beyond this point.
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_command(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// I'm doing this wrong. D: I need to write a proper string library.
	arg2 = arg1 + strlen(arg1) + 1 + (command - commandCopy);
	
	// Find var to set.
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	// Set var's script string.
	error = str2_copyMalloc(&var1->script, arg2);
	if (error) {
		goto cleanup_l;
	}
	
	// Convert ',' to ';'.
	str2_replaceChar(var1->script, ',', ';');
	
	// Set callback.
	var1->callback = cfg2_callback_callbackScript;
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_delete(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	// At this point, var->command has been set to the command string. Nothing else has been modified.
	
	// // First, let's get rid of var->name in the string.
	// char *start = strchr(var->command, ' ') + 1;
	// int offset = start - var->command;
	char *arg1 = NULL, *arg2 = NULL;
	cfg2_var_t *var1 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// // Delete var->name + ' '.
	// for (int i = 0; i < strlen(var->command) + 1 - offset; i++) {
	// 	var->command[i] = var->command[i + offset];
	// }
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 != NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Find variable.
	var1 = cfg2_findVar(arg1);
	if (var1 == NULL) {
		warning("Variable \"%s\" does not exist.", arg1);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Delete variable.
	error = cfg2_deleteVariable(var1);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

#ifdef CLIENT

int cfg2_callback_bind(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}
	
	char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL;
	cfg2_var_t *var2 = NULL, *var3 = NULL;
	
	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(commandCopy, command);
	
	// Now execute the command.
	arg1 = strtok(commandCopy, " ");
	if (arg1 == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		warning("Bad syntax for command \"%s\". (2)", var->name);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	
	arg3 = strtok(NULL, " ");
	if (arg3 != NULL) {
		arg4 = strtok(NULL, " ");
		if (arg4 != NULL) {
			warning("Bad syntax for command \"%s\". (4)", var->name);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	// Make sure variables exist. Functions can be added later since we are only storing the variable names and not the scripts themselves.
	var2 = cfg2_findVar(arg2);
	if (var2 == NULL) {
		warning("Variable \"%s\" does not exist.", arg2);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	if (arg3 != NULL) {
		var3 = cfg2_findVar(arg3);
		if (var3 == NULL) {
			warning("Variable \"%s\" does not exist.", arg2);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	// Bind the command(s).
	// arg3 can handle a NULL.
	error = input_bind(arg1, arg2, arg3);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	free(commandCopy);
	
	return error;
}

int cfg2_callback_bindMouse(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int e = ERR_OK;

	// Prevent execution on creation.
	static bool created = false;
	if (!created) {
		created = true;
		return ERR_OK;
	}

	uint8_t *callbackName = NULL;
	cfg2_var_t *var2 = NULL, *var3 = NULL;

	char *commandCopy = malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	strcpy(commandCopy, command);

	callbackName = strtok(commandCopy, " ");
	if (callbackName == NULL) {
		warning("Bad syntax for command \"%s\". (1)", var->name);
		e = ERR_GENERIC;
		goto cleanup;
	}

	// Bind the Lua callback.
	e = input_bindMouse(callbackName);
	if (e) goto cleanup;
	
 cleanup:
	free(commandCopy);
	return e;
}
        
#endif

/* cfg2 variable callbacks */
/* ======================= */

int cfg2_callback_maxRecursion(cfg2_var_t *var, const char *command, lua_State *luaState) {
	
	if (var->integer <= 0) {
		var->integer = 1;
		error("%s is negative or zero. Setting to %i", var->name, var->integer);
	}
	
	g_cfg2.maxRecursion = var->integer;
	
	return ERR_OK;
}

/* cfg2 variable callbacks */
/* ======================= */

int cfg2_callback_callbackScript(cfg2_var_t *var, const char *command, lua_State *luaState) {
	return cfg2_execString(var->script, luaState, NULL);
}

/* cfg2 functions */
/* ============== */


int cfg2_printVar(cfg2_var_t *var, const char *tag) {
	
	if (g_cfg2.adminLevel < var->permissionRead) {
		warning("Permission level not high enough to read variable \"%s\".", var->name);
		return ERR_OK;
	}
	
	if (tag != NULL) {
		printf(COLOR_CYAN"%s: ", tag);
	}
	
	switch (var->type) {
		case cfg2_var_type_none:
			printf(COLOR_BLUE"n[%s]"COLOR_NORMAL"\n", var->name);
			break;
		case cfg2_var_type_vector:
			printf(COLOR_BLUE"v[%s]"COLOR_CYAN" %f"COLOR_NORMAL"\n", var->name, var->vector);
			break;
		case cfg2_var_type_integer:
			printf(COLOR_BLUE"i[%s]"COLOR_CYAN" %i"COLOR_NORMAL"\n", var->name, var->integer);
			break;
		case cfg2_var_type_string:
			printf(COLOR_BLUE"s[%s]"COLOR_CYAN" \"%s\""COLOR_NORMAL"\n", var->name, var->string);
			break;
		default:
			log_error(__func__, "Can't happen");
			return ERR_GENERIC;
	}
	return ERR_OK;
}

int cfg2_createVariable(cfg2_var_t *var, const char *name, cfg2_var_type_t type, cfg2_admin_t adminLevel) {
	int error = ERR_CRITICAL;

	g_cfg2.vars_length++;

	// Create the variable. This is rarely done, so we won't worry about malloc speeds.
	g_cfg2.vars = realloc(g_cfg2.vars, g_cfg2.vars_length * sizeof(cfg2_var_t));
	if (g_cfg2.vars == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	var = &g_cfg2.vars[g_cfg2.vars_length - 1];
	
	memset(var, 0, sizeof(cfg2_var_t));
	
	var->callback = NULL;
	var->integer = 0;
	var->permissionRead = adminLevel;
	var->permissionWrite = adminLevel;
	var->permissionDelete = adminLevel;
	var->permissionCallback = adminLevel;
	var->string = NULL;
	var->type = type;
	var->vector = 0;
	
	var->name = malloc((strlen(name) + 1) * sizeof(char));
	if (var->name == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	strcpy(var->name, name);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

int cfg2_deleteVariable(cfg2_var_t *var) {
	int error = ERR_CRITICAL;
	
	const int varIndex = var - g_cfg2.vars;
	
	if (g_cfg2.adminLevel < var->permissionDelete) {
		warning("Permissions not high enough to delete variable %s.", var->name);
		error = ERR_OK;
		goto cleanup_l;
	}
	
	// Shrink list, starting by deleting the chosen variable.
	for (int i = varIndex; i < g_cfg2.vars_length - 1; i++) {
		g_cfg2.vars[i] = g_cfg2.vars[i + 1];
	}
	
	--g_cfg2.vars_length;
	g_cfg2.vars = realloc(g_cfg2.vars, g_cfg2.vars_length * sizeof(cfg2_var_t));
	if (g_cfg2.vars == NULL) {
		// We are freeing some memory. Is it possible to run out of memory like this?
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	var = NULL;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

cfg2_var_t *cfg2_findVar(const char *name) {
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		if (!strcmp(g_cfg2.vars[i].name, name)) {
			return &g_cfg2.vars[i];
		}
	}
	return NULL;
}

int cfg2_setVariable(cfg2_var_t *var, const char *value, lua_State *luaState, const char *tag) {
	int error = ERR_CRITICAL;
	
	if (strcmp(value, "") || ((var->type == cfg2_var_type_none) && (var->callback != NULL))) {
		// <variable> <value>
		
		if (g_cfg2.adminLevel < var->permissionWrite) {
			warning("Permissions not high enough to set variable %s.", var->name);
			error = ERR_OK;
			goto cleanup_l;
		}
		
		switch (var->type) {
		case cfg2_var_type_none:
			// Ignore it. This is a command.
			break;
		case cfg2_var_type_vector:
#ifdef DOUBLE_VEC
			var->vector = strtod(value, NULL);
#else
			var->vector = strtof(value, NULL);
#endif
			break;
		case cfg2_var_type_integer:
			var->integer = strtol(value, NULL, 10);
			break;
		case cfg2_var_type_string:
			var->string = realloc(var->string, (strlen(value) + 1) * sizeof(char));
			if (var->string == NULL) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			strcpy(var->string, value);
			break;
		default:
			critical_error("Bad variable type %i.", var->type);
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		
		// Run callback.
		if (var->callback != NULL) {
			error = var->callback(var, value, luaState);
			if (error) {
				goto cleanup_l;
			}
		}
	}
	else {
		// <variable>
		if (g_cfg2.adminLevel < var->permissionRead) {
			warning("Permissions not high enough to read variable %s.", var->name);
			error = ERR_OK;
			goto cleanup_l;
		}
		
		cfg2_printVar(var, tag);
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void cfg2_getInteger(int *value, cfg2_var_t *var) {
	value = NULL;
	if (g_cfg2.adminLevel < var->permissionRead) {
		warning("Permissions not high enough to read variable %s.", var->name);
		return;
	}
	value = &var->integer;
}

void cfg2_getVector(vec_t *value, cfg2_var_t *var) {
	value = NULL;
	if (g_cfg2.adminLevel < var->permissionRead) {
		warning("Permissions not high enough to read variable %s.", var->name);
		return;
	}
	value = &var->vector;
}

void cfg2_getInt(char *value, cfg2_var_t *var) {
	value = NULL;
	if (g_cfg2.adminLevel < var->permissionRead) {
		warning("Permissions not high enough to read variable %s.", var->name);
		return;
	}
	value = var->string;
}

void cfg2_setCallback(cfg2_var_t *var, int (*callback)(cfg2_var_t *var, const char *command, lua_State *luaState)) {

	var->callback = NULL;
	
	if (g_cfg2.adminLevel < var->permissionCallback) {
		warning("Permissions not high enough to set callback for variable %s.", var->name);
		return;
	}
	
	var->callback = callback;
}

void cfg2_getCallback(int (*callback)(cfg2_var_t *var, const char *command, lua_State *luaState), cfg2_var_t *var) {

	if (g_cfg2.adminLevel < var->permissionCallback) {
		warning("Permissions not high enough to read callback for variable %s.", var->name);
		return;
	}
	
	callback = var->callback;
}

void cfg2_init(lua_State *luaState) {
	g_cfg2.adminLevel = cfg2_admin_supervisor;
	g_cfg2.adminLevelDisguise = cfg2_admin_supervisor;
	g_cfg2.quit = false;
	g_cfg2.vars = NULL;
	g_cfg2.vars_length = 0;
	g_cfg2.maxRecursion = 0;
	g_cfg2.recursionDepth = 0;
}

/* cfg2_createVariables
Create variables from a list.
*/
int cfg2_createVariables(const cfg2_var_init_t *varInit, lua_State *luaState) {
	error = ERR_CRITICAL;
	
	int length;
	
	// Find length of init array.
	for (length = 0; varInit[length].name != NULL; length++);
	
	// Extend the variable list.
	g_cfg2.vars = realloc(g_cfg2.vars, (g_cfg2.vars_length + length) * sizeof(cfg2_var_t));
	if (g_cfg2.vars == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Write the variables to the list.
	for (int i = 0; i < length; i++) {
		// Clear each variable to prevent Valgrind from complaining.
		memset(&g_cfg2.vars[i + g_cfg2.vars_length], 0, sizeof(cfg2_var_t));
		// Copy the structure.
		g_cfg2.vars[i + g_cfg2.vars_length].callback = varInit[i].callback;
		g_cfg2.vars[i + g_cfg2.vars_length].integer = varInit[i].integer;
		g_cfg2.vars[i + g_cfg2.vars_length].permissionCallback = varInit[i].permissionCallback;
		g_cfg2.vars[i + g_cfg2.vars_length].permissionDelete = varInit[i].permissionDelete;
		g_cfg2.vars[i + g_cfg2.vars_length].permissionRead = varInit[i].permissionRead;
		g_cfg2.vars[i + g_cfg2.vars_length].permissionWrite = varInit[i].permissionWrite;
		g_cfg2.vars[i + g_cfg2.vars_length].type = varInit[i].type;
		g_cfg2.vars[i + g_cfg2.vars_length].vector = varInit[i].vector;
		g_cfg2.vars[i + g_cfg2.vars_length].script = NULL;
		// Copy the strings, since free won't like them.
		
		g_cfg2.vars[i + g_cfg2.vars_length].name = malloc((strlen(varInit[i].name) + 1) * sizeof(char));
		if (g_cfg2.vars[i + g_cfg2.vars_length].name == NULL) {
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		strcpy(g_cfg2.vars[i + g_cfg2.vars_length].name, varInit[i].name);
		
		if (varInit[i].string == NULL) {
			g_cfg2.vars[i + g_cfg2.vars_length].string = NULL;
		}
		else {
			g_cfg2.vars[i + g_cfg2.vars_length].string = malloc((strlen(varInit[i].string) + 1) * sizeof(char));
			if (g_cfg2.vars[i + g_cfg2.vars_length].string == NULL) {
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
			strcpy(g_cfg2.vars[i + g_cfg2.vars_length].string, varInit[i].string);
		}
		
		// Set the remaining default values.
		g_cfg2.vars[i + g_cfg2.vars_length].frequency = 0;
		
		// Run callback.
		if (g_cfg2.vars[i + g_cfg2.vars_length].callback != NULL) {
			error = g_cfg2.vars[i + g_cfg2.vars_length].callback(&g_cfg2.vars[i + g_cfg2.vars_length], "", luaState);
			if (error >= ERR_CRITICAL) {
				goto cleanup_l;
			}
		}
	}
	
	// Allow functions to use the variables.
	g_cfg2.vars_length += length;
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

void cfg2_free(void) {
	for (int i = 0; i < g_cfg2.vars_length; i++) {
		if (g_cfg2.vars[i].name) MEMORY_FREE(&g_cfg2.vars[i].name);
		if (g_cfg2.vars[i].string) MEMORY_FREE(&g_cfg2.vars[i].string);
		if (g_cfg2.vars[i].script) MEMORY_FREE(&g_cfg2.vars[i].script);
	}
	MEMORY_FREE(&g_cfg2.vars);
	g_cfg2.vars_length = 0;
}

int cfg2_execString(const char *line, lua_State *luaState, const char *tag) {
	int error = ERR_CRITICAL;
	
	cfg2_var_t *var;
	
	char *lineCopy = NULL;
	char **commands = NULL;
	char *varName = NULL;
	char *value = NULL;
	size_t numCommands = 0;
	char *strtokPtr = NULL;
	size_t commandLength;
	
	if (g_cfg2.recursionDepth >= g_cfg2.maxRecursion) {
		error("Reached limit of %i recursions.", g_cfg2.recursionDepth);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = str2_copyMalloc(&lineCopy, line);
	if (error) {
		goto cleanup_l;
	}
	
	while (1) {
		strtokPtr = strtok(numCommands ? NULL : lineCopy, ";");
		if (strtokPtr == NULL) {
			break;
		}
		else {
			numCommands++;
		}
		
		commands = realloc(commands, numCommands * sizeof(char *));
		if (commands == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		commands[numCommands - 1] = strtokPtr;
	}
	
	for (int i = 0; i < numCommands; i++) {
		str2_removeWhitespace(commands[i], "melt");
		
		commandLength = strlen(commands[i]);
		
		varName = strtok(commands[i], " ");
		if (varName == NULL) {
			error("Bad syntax.", "");
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		value = varName + strlen(varName) + 1;
		if (value - commands[i] > strlen(line)) {
			value = NULL;
		}
		// else {
			// value.length = strlen(value.value);
		// }
		
		// index = string_index_of(line, 0, ' ');
		// string_substring(&varName, line, 0, index - 0);
		// if (index >= 0) {
		// 	string_substring(&value, line, index + 1, -1);
		// }
		// else {
		// 	str2_copy_c(&value, "");
		// }
		
		var = cfg2_findVar(varName);
		if (var == NULL) {
			error("Variable \"%s\" does not exist.", varName);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		
		// Curse you strtok.
		if ((value == NULL) || (value > commands[i] + commandLength)) {
			value = NULL;
			error = str2_copyMalloc(&value, "");
			if (error) {
				goto cleanup_l;
			}
			error = cfg2_setVariable(var, value, luaState, tag);
			MEMORY_FREE(&value);
		}
		else {
			error = cfg2_setVariable(var, value, luaState, tag);
		}
		if (error) {
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	MEMORY_FREE(&commands);
	MEMORY_FREE(&lineCopy);
	// MEMORY_FREE(&varName);
	// MEMORY_FREE(&value);
	
	return error;
}

int cfg2_execFile(const char *filepath, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	PHYSFS_File *vfsFile;
	char *line = NULL;
	char **lines = NULL;
	size_t lines_length = 0;
	char *fileText = NULL;
	size_t fileText_length;
	cfg2_var_t *v_quiet;
	int quiet = 0;
	
	vfsFile = PHYSFS_openRead(filepath);
	if (vfsFile == NULL) {
		error("Could not open file %s", filepath);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	fileText_length = PHYSFS_fileLength(vfsFile);
	fileText = calloc(fileText_length + 1, sizeof(char));
	if (fileText == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	PHYSFS_readBytes(vfsFile, fileText, fileText_length);
	
	v_quiet = cfg2_findVar(CFG_RUN_QUIET);
	if (v_quiet == NULL) {
		warning(CFG_RUN_QUIET" is undefined.", "");
	}
	else if (v_quiet->type != cfg2_var_type_integer) {
		warning(CFG_RUN_QUIET" is not an integer.", "");
	}
	else {
		quiet = v_quiet->integer;
	}
	
	if (!(quiet & 1)) {
		info("Exec'ing \"%s\"", filepath);
	}
	
	error = str2_tokenizeMalloc(&lines, &lines_length, fileText, "\n");
	if (error) {
		goto cleanup_l;
	}
	
	for (int linenumber = 1; linenumber - 1 < lines_length; linenumber++) {
		
		line = lines[linenumber - 1];
		
		if (line == NULL) {
			break;
		}
		
		/* Remove comments and unnecessary whitespace. */
		str2_removeLineComments(line, "#");
		str2_removeWhitespace(line, "melt");
		/* Convert remaining whitespace to spaces */
		for (int i = 0; i < strlen(line); i++) {
			if (isspace(line[i])) {
				line[i] = ' ';
			}
		}
		
		/* Discard empty lines. */
		if (strlen(line) == 0) {
			continue;
		}
		
		error = cfg2_execString(line, luaState, (quiet & 2) ? "" : filepath);
		if (error) {
			error("Error on line %i, recursion level %i", linenumber, g_cfg2.recursionDepth);
			break;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	
	PHYSFS_close(vfsFile);
	
	MEMORY_FREE(&fileText);
	if (lines != NULL) {
		MEMORY_FREE(lines);
	}
	MEMORY_FREE(&lines);
	
	return error;
}
