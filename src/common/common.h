
#ifndef COMMON_H
#define COMMON_H

#include "types.h"

#define ERR_OK          0
#define ERR_GENERIC     1
#define ERR_CRITICAL    2
#define ERR_OUTOFMEMORY 3
#define ERR_NULLPOINTER 4

// @TODO: Might want to change this to an actual variable.
#define ERR ((char*[15]) { \
	"OK", \
	"Generic error", \
	"Critical error", \
	"Out of memory" \
})

#define COLOR_NORMAL    "\x1B[0m"
#define COLOR_BLACK     "\x1B[30m"
#define COLOR_RED       "\x1B[31m"
#define COLOR_GREEN     "\x1B[32m"
#define COLOR_YELLOW    "\x1B[33m"
#define COLOR_BLUE      "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN      "\x1B[36m"
#define COLOR_WHITE     "\x1B[37m"

#define B_COLOR_BLACK     "\x1B[40m"
#define B_COLOR_RED       "\x1B[41m"
#define B_COLOR_GREEN     "\x1B[42m"
#define B_COLOR_YELLOW    "\x1B[43m"
#define B_COLOR_BLUE      "\x1B[44m"
#define B_COLOR_MAGENTA   "\x1B[45m"
#define B_COLOR_CYAN      "\x1B[46m"
#define B_COLOR_WHITE     "\x1B[47m"

#define AUTOEXEC    "autoexec.cfg"

#define ENGINE_MAN_NAME "Scott"

// Macros have completion support. Strings don't.
#define CFG_LUA_MAIN            "lua_main"
#define CFG_LUA_MAIN_DEFAULT            ""
#define CFG_WORKSPACE           "workspace"
#define CFG_WORKSPACE_DEFAULT           ""
#define CFG_PORT                "net_port"
#define CFG_PORT_DEFAULT                8099
#define CFG_IP_ADDRESS          "net_address"
#define CFG_IP_ADDRESS_DEFAULT          "127.0.0.1"
#define CFG_CONNECTION_TIMEOUT  "net_timeout"
#define CFG_CONNECTION_TIMEOUT_DEFAULT  10000   // Milliseconds
#define CFG_MAX_FRAMERATE       "max_framerate"
#define CFG_MAX_FRAMERATE_DEFAULT  20	// Milliseconds
#define CFG_MAX_CLIENTS         "max_clients"
#define CFG_MAX_CLIENTS_DEFAULT         MAX_CLIENTS
#define CFG_MAX_RECURSION       "max_recursion_depth"
#define CFG_MAX_RECURSION_DEFAULT       10
#define CFG_RUN_QUIET           "quiet"
#define CFG_RUN_QUIET_DEFAULT           0
#define CFG_HISTORY_LENGTH      "command_history_length"
#define CFG_HISTORY_LENGTH_DEFAULT      10
#define CFG_OPENGL_LOG_FILE     "opengl_log_file"
#define CFG_OPENGL_LOG_FILE_DEFAULT     "opengl.log"
#define CFG_LOG_LEVEL           "log_level"
#define CFG_LOG_LEVEL_DEFAULT           0
#define CFG_NAMED_PIPE          "named_pipe"
#define CFG_NAMED_PIPE_DEFAULT           0

#define MAIN_LUA_STARTUP_TIMEOUT    10000
#define MAIN_LUA_MAIN_TIMEOUT       100
#define MAIN_LUA_SHUTDOWN_TIMEOUT   1000

#define MAIN_LUA_DELTAT_NAME "deltaT"

extern int error;

extern const cfg2_var_init_t g_commonVarInit[];

int common_getTimeNs(long *ns);

int l_common_puts(lua_State*);

int MSB(int n);

void network_dumpBufferUint8(const uint8_t *buffer, size_t length);

#endif
