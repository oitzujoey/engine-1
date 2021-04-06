
#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

int terminal_runTerminalCommand(lua_State *luaState);
int terminal_initConsole(void);
int terminal_terminalInit(void);
void terminal_quitConsole(void);

int terminal_callback_updateCommandHistoryLength(cfg2_var_t *var, const char *command, lua_State *luaState);

#endif
