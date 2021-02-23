
#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <physfs.h>
#include "common.h"
#include "file.h"
#include "log.h"
#include "cfg2.h"
#include "str2.h"

vfs_t g_vfs;
char *g_workspace = NULL;
vfs_mods_t g_mods = {
	.mods = NULL,
	.mods_length = 0
};


/* Config commands */
/* =============== */

// Helper command for vfs_callback_loadMod.
static int vfs_PHYSFS_saveScriptFilesEnumerator(void *data, const char *origdir, const char *fname) {
	int error = PHYSFS_ENUM_ERROR;
	
	size_t file_length = 0;
	
	// origdir + / + fname + \0
	char *filepath = malloc((strlen(origdir) + strlen(fname) + 2) * sizeof(char));
	if (filepath == NULL) {
		critical_error("Out of memory.", "");
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}
	
	sprintf(filepath, "%s/%s", origdir, fname);
	
	PHYSFS_Stat stat;
	error = PHYSFS_stat(filepath, &stat);
	if (!error) {
		error("Could not get stats of file \"%s\": %s", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}
	
	if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
		error = PHYSFS_enumerate(filepath, vfs_PHYSFS_saveScriptFilesEnumerator, NULL);
		if (!error) {
			error("Could not enumerate files in directory \"%s\": %s", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
			error = PHYSFS_ENUM_ERROR;
			goto cleanup_l;
		}
	}
	
	// Ignore files that aren't executable.
	const char *ext = file_getExtension(fname);
	if ((ext == NULL) || (strcmp(ext, "cfg") && strcmp(ext, "lua"))) {
		error = PHYSFS_ENUM_OK;
		goto cleanup_l;
	}
	
	// Stick the file in the mod structure.
	
	info("Found %s", filepath);
	
	g_mods.mods[g_mods.mods_length - 1].files_length++;
	int files_length = g_mods.mods[g_mods.mods_length - 1].files_length;
	char ***filenames = &g_mods.mods[g_mods.mods_length - 1].filenames;
	char ***files = &g_mods.mods[g_mods.mods_length - 1].files;
	
	*filenames = realloc(*filenames, files_length * sizeof(char *));
	*files = realloc(*files, files_length * sizeof(char *));
	if ((*filenames == NULL) || (*files == NULL)) {
		critical_error("Out of memory.", "");
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}
	
	error = str2_copyMalloc((*filenames)[files_length - 1], filepath);
	if (error) {
		critical_error("Out of memory.", "");
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}

	PHYSFS_File *file = PHYSFS_openRead(filepath);
	
	(*files)[files_length - 1] = calloc((PHYSFS_fileLength(file) + 1), sizeof(char));
	file_length = PHYSFS_fileLength(file);
	if ((*files)[files_length - 1] == NULL) {
		critical_error("Out of memory.", "");
		error = PHYSFS_ENUM_ERROR;
		goto cleanupPhys_l;
	}
	
	error = PHYSFS_readBytes(file, (*files)[files_length - 1], file_length);
	if (error < 0) {
		error("Could not read from file \"%s\": %s", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = PHYSFS_ENUM_ERROR;
		goto cleanupPhys_l;
	}
	
	error = PHYSFS_ENUM_OK;
	cleanupPhys_l:
	
	PHYSFS_close(file);
	
	cleanup_l:
	
	insane_free(filepath);
	
    return error;
}

int vfs_callback_loadMod(cfg2_var_t *var, const char *command) {
	int error = ERR_CRITICAL;
	
	if (!strcmp(command, "")) {
		return ERR_OK;
	}
	
	// Add directory to search path.
	
	// All mods will be mounted to the root directory.
	error = PHYSFS_mount(command, "", true);
	if (!error) {
		error("Could not add directory \"%s\" to the search path: %s", command, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}
	
	// Make space for the next mod.
	
	g_mods.mods_length++;
	g_mods.mods = realloc(g_mods.mods, g_mods.mods_length * sizeof(vfs_mod_t));
	if (g_mods.mods == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	g_mods.mods[g_mods.mods_length - 1].files = NULL;
	g_mods.mods[g_mods.mods_length - 1].filenames = NULL;
	g_mods.mods[g_mods.mods_length - 1].files_length = 0;
	
	// Load files from mod directory.
	
	info("Searching for scripts in mod \"%s\"", command);
	
	error = PHYSFS_enumerate(command, vfs_PHYSFS_saveScriptFilesEnumerator, NULL);
	if (!error) {
		error("Could not enumerate files in directory \"%s\": %s", command, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}


/* Config callbacks */
/* ================ */

int vfs_callback_setWorkspace(cfg2_var_t *var, const char *command) {
	g_workspace = var->string;
	if (strcmp(g_workspace, "") && !PHYSFS_setWriteDir(g_workspace)) {
		error("Could not set workspace to \"%s\": %s", g_workspace, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return ERR_GENERIC;
	}
	return ERR_OK;
}

/* VFS helper functions */
/* ==================== */

// int vfs_execAutoexec() {
	
// 	/*  Here, we cheat.
// 		Instead of doing a fancy merge, we just run one autoexec file after
// 		the other and hope for the best.
// 	*/
	
	
// }
