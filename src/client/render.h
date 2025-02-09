
#ifndef RENDER_H
#define RENDER_H

#include "lua.h"
#include "../common/cfg2.h"
#include "../common/types.h"

extern SDL_DisplayMode g_displayMode;
extern char *g_openglLogFileName;

int render_callback_updateLogFileName(cfg2_var_t *var);

int render_initOpenGL(void);
int render(entity_t *entity);

#endif
