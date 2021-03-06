
#ifndef LOG_H
#define LOG_H

#include "types.h"
#include "common.h"

int log_callback_updateLogLevel(cfg2_var_t *var, const char *command, lua_State *luaState);

void log_info(const char *function, const char *fmt, ...);
void log_warning(const char *function, const char *fmt, ...);
void log_error(const char *function, const char *fmt, ...);
void log_critical_error(const char *function, const char *fmt, ...);
void log_outOfMemory(const char *function);

#define info(fmt, ...) log_info(__func__, fmt, __VA_ARGS__)
#define warning(fmt, ...) log_warning(__func__, fmt, __VA_ARGS__)
#define error(fmt, ...) log_error(__func__, fmt, __VA_ARGS__)
#define critical_error(fmt, ...) log_critical_error(__func__, fmt, __VA_ARGS__)
#define outOfMemory() log_outOfMemory(__func__)

int l_log_info(lua_State *l);
int l_log_warning(lua_State *l);
int l_log_error(lua_State *l);
int l_log_critical_error(lua_State *l);

#endif
