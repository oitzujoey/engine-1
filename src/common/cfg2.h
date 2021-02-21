
#ifndef CFG2_H
#define CFG2_H

#include "types.h"

extern cfg2_t g_cfg2;

int cfg2_callback_set(cfg2_var_t *var, const char *command);
int cfg2_callback_ifdef(cfg2_var_t *var, const char *command);
int cfg2_callback_exec(cfg2_var_t *var, const char *command);
int cfg2_callback_create(cfg2_var_t *var, const char *command);
int cfg2_callback_quit(cfg2_var_t *var, const char *command);
int cfg2_callback_vars(cfg2_var_t *var, const char *command);
int cfg2_callback_copy(cfg2_var_t *var, const char *command);
int cfg2_callback_adminLevel(cfg2_var_t *var, const char *command);
int cfg2_callback_su(cfg2_var_t *var, const char *command);
int cfg2_callback_add(cfg2_var_t *var, const char *command);
int cfg2_callback_sub(cfg2_var_t *var, const char *command);
int cfg2_callback_if(cfg2_var_t *var, const char *command);

int cfg2_callback_maxRecursion(cfg2_var_t *var, const char *command);

int cfg2_printVar(cfg2_var_t *var, const char *tag);
int cfg2_createVariable(cfg2_var_t *var, const char *name, cfg2_var_type_t type, cfg2_admin_t adminLevel);
cfg2_var_t *cfg2_findVar(const char *name);
int cfg2_setVariable(cfg2_var_t *var, const char *value, const char *tag);
void cfg2_init(lua_State *luaState);
int cfg2_createVariables(const cfg2_var_init_t *varInit);
void cfg2_free(void);
int cfg2_execString(const string_t *line, const char *tag);
int cfg2_execFile(const char *filepath);

#endif
