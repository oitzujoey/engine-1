
#ifndef RENDER_H
#define RENDER_H

#include "lua.h"
#include "../common/cfg.h"

extern char *g_openglLogFileName;

int render_handle_updateLogFileName(cfg_var_t *var);

int render_initOpenGL(void);
int render(lua_State*);

#endif
