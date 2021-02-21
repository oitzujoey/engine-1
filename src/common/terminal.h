
#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

int terminal_runTerminalCommand(void);
int terminal_initConsole(void);
int terminal_terminalInit(void);
void terminal_quitConsole(void);

int terminal_callback_updateCommandHistoryLength(cfg2_var_t *var, const char *command);

#endif
