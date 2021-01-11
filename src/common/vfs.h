
#ifndef VFS_H
#define VFS_H

#include <lua.h>
#include "str.h"
#include "cfg.h"

// typedef struct {

// } vfs_file_t;

typedef enum vfs_type_e {
	vfs_type_directory,
	vfs_type_zip
} vfs_type_t;

// typedef struct {
	
// } vfs_file_t;

typedef struct {
	string_t path;
	vfs_type_t workspace_type;
} vfs_t;

extern vfs_t g_vfs;
extern char *g_workspace;

int vfs_handle_setWorkspace(cfg_var_t *var);

int vfs_getFileText(vfs_t *vfs, string_t *fileText, const string_t *workspace_path);
int l_vfs_getFileText(lua_State *luaState);
int vfs_init(vfs_t *vfs, const string_t *path);
void vfs_free(vfs_t *vfs);

#endif
