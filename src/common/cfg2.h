
#ifndef CFG2_H
#define CFG2_H

#include "types.h"

extern cfg2_t g_cfg2;

int cfg2_callback_set(cfg2_var_t *var, const char *command, lua_State *luaState);
int cfg2_callback_ifdef(cfg2_var_t *var, const char *command, lua_State *luaState);
int cfg2_callback_exec(cfg2_var_t *var, const char *command, lua_State *luaState);

int cfg2_callback_maxRecursion(cfg2_var_t *var, const char *command, lua_State *luaState);

int cfg2_printVar(cfg2_var_t *var, const char *tag);
cfg2_var_t *cfg2_findVar(const char *name);
void cfg2_init(lua_State *luaState);
int cfg2_createVariables(const cfg2_var_init_t *varInit);
void cfg2_free(void);
int cfg2_execString(const string_t *line, const char *tag, const int recursionDepth);
int cfg2_execFile(const char *filepath, const int recursionDepth);

#endif
