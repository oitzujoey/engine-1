
#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "common.h"
#include "file.h"
#include "log.h"
#include "cfg.h"

vfs_t g_vfs;
char *g_workspace;

int vfs_handle_setWorkspace(cfg_var_t *var) {
	g_workspace = var->string.value;
	return ERR_OK;
}

int vfs_getFileText(vfs_t *vfs, string_t *fileText, const string_t * const workspace_path) {
	
	int localError = ERR_OK;
	
	if (vfs->workspace_type == vfs_type_directory) {
		string_t path;
		string_init(&path);
		string_copy(&path, &vfs->path);
		file_concatenatePath(&path, workspace_path);

		if (file_pathIsInDirectory(&path, &vfs->path)) {
			free(fileText->value);
			fileText->value = file_getText(path.value);
			if (fileText->value == NULL) {
				log_warning(__func__, "Could not open file \"%s\"", path.value);
				localError = ERR_GENERIC;
				goto dirCleanup_l;
			}
			string_normalize(fileText);
		}
		else {
			log_error(__func__, "Path \"%s\" leads outside of workspace.", workspace_path->value);
		}

		dirCleanup_l:

		string_free(&path);
		
		return localError;
	}
	return ERR_GENERIC;
}

int l_vfs_getFileText(lua_State *luaState) {
	
	string_t fileText;
	int localError = 0;
	
	string_init(&fileText);
	
	localError = vfs_getFileText(&g_vfs, &fileText, string_const(lua_tostring(luaState, 1)));
	if (!localError) {
		lua_pushstring(luaState, fileText.value);
	}
	
	string_free(&fileText);
	
	if (localError) {
		return 0;
	}
	return 1;
}

int vfs_init(vfs_t *vfs, const string_t *path) {

	const char *extension;

	string_copy(&vfs->path, path);
	
	extension = file_getExtension(path->value);
	
	if (file_isDirectory(path->value)) {
		vfs->workspace_type = vfs_type_directory;
	}
	else if (file_isRegularFile(path->value)) {
		if (strcmp(extension, "zip")) {
			vfs->workspace_type = vfs_type_zip;

			log_error(__func__, "Zip is not implemented. Get out there and fix it.");
			return ERR_GENERIC;
		}
		else {
			log_error(__func__, "Unsupported file type");
			return ERR_GENERIC;
		}
	}
	else {
		log_error(__func__, "Unsupported file type");
		return ERR_GENERIC;
	}
	
	if (error) {
		return ERR_GENERIC;
	}
	return ERR_OK;
}

void vfs_free(vfs_t *vfs) {
	string_free(&vfs->path);
}
