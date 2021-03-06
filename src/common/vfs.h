
#ifndef VFS_H
#define VFS_H

#include "types.h"

extern char *g_workspace;

int vfs_callback_loadMod(cfg2_var_t *var, const char *command, lua_State *luaState);

int vfs_callback_setWorkspace(cfg2_var_t *var, const char *command, lua_State *luaState);

int vfs_getFileText(char **fileText, const char *path);
int vfs_getFileContents_malloc(uint8_t **fileContents, PHYSFS_sint64 *fileContents_length, const char *path);
// int l_vfs_getFileText(lua_State *luaState);
int vfs_init(vfs_t *vfs, const char *path);
void vfs_free(vfs_t *vfs);

#endif
