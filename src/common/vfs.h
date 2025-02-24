
#ifndef VFS_H
#define VFS_H

#include "types.h"
#include "str4.h"

extern Str4 g_workspace;
extern Str4 g_game;
extern vfs_mods_t g_mods;

int vfs_callback_loadGame(cfg2_var_t *var, const char *command, lua_State *luaState);

int vfs_callback_setWorkspace(cfg2_var_t *var, const char *command, lua_State *luaState);

// Get contents of a text file.
int vfs_getFileText(char **fileText, const char *path);
// Get contents of a binary file. `Path` is null terminated. 2025-02-23: Probably best to use this instead of `vfs_getFileText`?
int vfs_getFileContents_malloc(uint8_t **fileContents, PHYSFS_sint64 *fileContents_length, const uint8_t *path);
int vfs_init(uint8_t *argv0);
/* void vfs_free(vfs_t *vfs); */

#endif
