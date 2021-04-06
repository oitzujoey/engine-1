
#include "common.h"
#include <stdio.h>
#include <math.h>
#include "cfg2.h"
#include "terminal.h"
#include "vfs.h"
#include "network.h"
#include "log.h"

int error;

const cfg2_var_init_t g_commonVarInit[] = {
	// Commands
	{
		.name = "set",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_set
	},
	{
		.name = "ifdef",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_ifdef
	},
	{
		.name = "exec",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_exec
	},
	{
		.name = "create",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_create
	},
	{
		.name = "quit",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_quit
	},
	{
		.name = "exit",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_quit
	},
	{
		.name = "vars",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_vars
	},
	{
		.name = "copy",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_copy
	},
	{
		.name = "=",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_copy
	},
	{
		.name = "adminLevel",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_adminLevel
	},
	{
		.name = "su",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_su
	},
	{
		.name = "suDisguise",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_suDisguise
	},
	{
		.name = "add",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_add
	},
	{
		.name = "+",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_add
	},
	{
		.name = "sub",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_sub
	},
	{
		.name = "-",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_sub
	},
	{
		.name = "if",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_if
	},
	{
		.name = "ifn",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_ifn
	},
	{
		.name = "load_mod",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = vfs_callback_loadMod
	},
	{
		.name = "command",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_command
	},
	{
		.name = "delete",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_delete
	},
	// Variables
#ifdef LINUX
	{
		.name = "linux",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = NULL
	},
#endif
#ifdef WINDOWS
	{
		.name = "windows",
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_none,
		.permissionRead = cfg2_admin_supervisor,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = NULL
	},
#endif
	{
		.name = CFG_LUA_MAIN,
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_string,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = NULL
	},
	{
		.name = CFG_WORKSPACE,
		.vector = 0,
		.integer = 0,
		.string = "",
		.type = cfg2_var_type_string,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_supervisor,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = vfs_callback_setWorkspace
	},
	{
		.name = CFG_CONNECTION_TIMEOUT,
		.vector = 0,
		.integer = CFG_CONNECTION_TIMEOUT_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_administrator,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = network_callback_connectionTimeout
	},
	{
		.name = CFG_MAX_RECURSION,
		.vector = 0,
		.integer = CFG_MAX_RECURSION_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_administrator,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = cfg2_callback_maxRecursion
	},
	{
		.name = CFG_RUN_QUIET,
		.vector = 0,
		.integer = CFG_RUN_QUIET_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_administrator,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = NULL
	},
	{
		.name = CFG_HISTORY_LENGTH,
		.vector = 0,
		.integer = CFG_HISTORY_LENGTH_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = terminal_callback_updateCommandHistoryLength
	},
	{
		.name = CFG_LOG_LEVEL,
		.vector = 0,
		.integer = CFG_LOG_LEVEL_DEFAULT,
		.string = "",
		.type = cfg2_var_type_integer,
		.permissionRead = cfg2_admin_game,
		.permissionWrite = cfg2_admin_game,
		.permissionDelete = cfg2_admin_supervisor,
		.permissionCallback = cfg2_admin_supervisor,
		.callback = log_callback_updateLogLevel
	},
	{
		.name = NULL,
		.vector = 0,
		.integer = 0,
		.string = NULL,
		.type = 0,
		.permissionRead = 0,
		.permissionWrite = 0,
		.permissionDelete = 0,
		.permissionCallback = 0,
		.callback = NULL
	}
};

int common_getTimeNs(long *ns) {
	int error = ERR_CRITICAL;
	
// #ifdef LINUX
// 	struct timespec timespec;
	
// 	error = clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
// 	if (error < 0) {
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
// 	*ns = timespec.tv_nsec;
// #else
// #error "common_getTimeNs has not been rewritten for this platform."
// #endif
	error = ERR_OK;
	// cleanup_l:
	return error;
}

int l_common_puts(lua_State *Lua) {
    printf(COLOR_CYAN"Lua {"COLOR_NORMAL"%s"COLOR_CYAN"}"COLOR_NORMAL"\n", lua_tostring(Lua, 1));
    return 0;
}

int MSB(int n) {
	int i;
	
	for (i = 0; (1<<i) <= n; i++);
	
	return i - 1;
}
