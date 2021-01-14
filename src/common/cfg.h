
#ifndef CFG_H
#define CFG_H

#include <stdbool.h>
#include "str.h"
#include "common.h"

typedef enum cfg_var_type_e {
	none,
	vector,
	integer,
	string
} cfg_var_type_t;

#define CFG_VAR_FLAG_NONE       (0)
#define CFG_VAR_FLAG_READ       (1<<0)
#define CFG_VAR_FLAG_WRITE      (1<<1)
#define CFG_VAR_FLAG_DELETE     (1<<2)
#define CFG_VAR_FLAG_PERMISSION (1<<3)
#define CFG_VAR_FLAG_PROTECTED  (1<<4)
#define CFG_VAR_PERMISSION_ALL          ( \
											CFG_VAR_FLAG_READ       | \
											CFG_VAR_FLAG_WRITE      | \
											CFG_VAR_FLAG_DELETE     | \
											CFG_VAR_FLAG_PERMISSION   \
										)

typedef struct cfg_var_s {
	char *name;
	vec_t vector;
	int integer;
	string_t string;
	cfg_var_type_t type;
	unsigned int permissions;
	int (*handle)(struct cfg_var_s *var);
} cfg_var_t;

typedef struct {
	char *name;
	vec_t vector;
	int integer;
	char *string;
	cfg_var_type_t type;
	unsigned int permissions;
	int (*handle)(cfg_var_t *var);
} cfg_var_init_t;

typedef struct {
	bool lock;
	bool quit;
	cfg_var_t *vars;
	int vars_length;
} cfg_t;

// Macros have completion support. Strings don't.
#define CFG_PORT                "net_port"
#define CFG_PORT_DEFAULT                8099
#define CFG_IP_ADDRESS          "net_address"
#define CFG_CONNECTION_TIMEOUT  "net_timeout"
#define CFG_CONNECTION_TIMEOUT_DEFAULT  10000   // Milliseconds
#define CFG_MAX_CLIENTS         "max_clients"
#define CFG_MAX_CLIENTS_DEFAULT         MAX_CLIENTS
#define CFG_MAX_RECURSION       "max_recursion_depth"
#define CFG_MAX_RECURSION_DEFAULT       10
#define CFG_RUN_QUIET           "quiet"
#define CFG_HISTORY_LENGTH      "command_history_length"
#define CFG_HISTORY_LENGTH_DEFAULT      10
#define CFG_OPENGL_LOG_FILE     "opengl_log_file"
#define CFG_OPENGL_LOG_FILE_DEFAULT     "opengl.log"

#define CFG_NUM_COMMANDS    20
typedef struct {
	char *commands[CFG_NUM_COMMANDS];
	int arguments_min[CFG_NUM_COMMANDS];
	int arguments_max[CFG_NUM_COMMANDS];
	int frequency[CFG_NUM_COMMANDS];
	int length;
} cfg_commandList_t;

extern cfg_t g_cfg;
extern string_t g_consoleCommand;

int cfg_initVars(const cfg_var_init_t *initCfgList);
cfg_var_t *cfg_addVarNone(const char *name);
cfg_var_t *cfg_addVarVector(const char *name, const vec_t value);
cfg_var_t *cfg_addVarInt(const char *name, const int value);
cfg_var_t *cfg_addVarString(const char *name, const char *value);
int cfg_setVarVector(cfg_var_t *var, const vec_t value);
int cfg_setVarInt(cfg_var_t *var, const int value);
int cfg_setVarString(cfg_var_t *var, const char *value);
int cfg_getVarVector(cfg_var_t *var, vec_t *value);
int cfg_getVarInt(cfg_var_t *var, int *value);
int cfg_getVarString(cfg_var_t *var, string_t **value);
cfg_var_t *cfg_findVar(const char *name);
int cfg_deleteVar(cfg_var_t *var);
int cfg_printVar(cfg_var_t *var, const char *tag);

/* tag is what is shown when the script prints a message. */
int cfg_execString(const string_t *line, const char *tag, const int recursionDepth);
int cfg_execFile(const char *filepath, const int recursionDepth);

void cfg_free(void);

int cfg_terminalInit(void);
int cfg_runTerminalCommand(void);

int cfg_addLineToHistory(const string_t *line);
int cfg_getHistoryLine(string_t *line, int *index);
int cfg_initConsole(void);
void cfg_quitConsole(void);

int cfg_handle_updateCommandHistoryLength(cfg_var_t *var);
int cfg_handle_maxRecursion(cfg_var_t *var);

#endif
