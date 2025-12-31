#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../physfs-3.0.2/src/physfs.h"
#include "common.h"
#include "file.h"
#include "log.h"
#include "cfg2.h"
#include "str2.h"
#include "memory.h"
#include "arena.h"


// @TODO: See how `g_mods` contains an array of mods? This was misguided and it should only have one mod.
Str4 g_workspace;
Str4 g_game;
vfs_mods_t g_mods = {
	.mods = NULL,
	.mods_length = 0
};


int vfs_init(uint8_t *argv0) {
	int e = ERR_OK;

	static Allocator allocator;
	allocator = allocator_create_stdlib();
	g_workspace = str4_create(&allocator);
	g_game = str4_create(&allocator);

	e = PHYSFS_init((char *) argv0);
	if (!e) {
		critical_error("Could not start PhysFS: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		e = ERR_CRITICAL;
		goto cleanup;
	}
	e = ERR_OK;

 cleanup: return e;
}


/* Config commands */
/* =============== */

// Helper command for vfs_callback_loadMod.
static int vfs_PHYSFS_saveScriptFilesEnumerator(void *data, const char *origdir, const char *fname) {
	int error = PHYSFS_ENUM_ERROR;
	
	size_t file_length = 0;
	
	// origdir + / + fname + \0
	char *filepath = malloc((strlen(origdir) + strlen(fname) + 2) * sizeof(char));
	if (filepath == NULL) {
		outOfMemory();
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
		outOfMemory();
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}
	
	// Initialize pointer to zero so that malloc doesn't choke. (Actually, it's Valgrind, but still technically correct.)
	(*filenames)[files_length - 1] = NULL;
	error = str2_copyMalloc(&(*filenames)[files_length - 1], filepath);
	if (error) {
		outOfMemory();
		error = PHYSFS_ENUM_ERROR;
		goto cleanup_l;
	}

	PHYSFS_File *file = PHYSFS_openRead(filepath);
	if (file == NULL) {
		error("Could not open file \"%s\": %s", filepath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = PHYSFS_ENUM_ERROR;
		goto cleanupPhys_l;
	}
	
	(*files)[files_length - 1] = calloc((PHYSFS_fileLength(file) + 1), sizeof(char));
	file_length = PHYSFS_fileLength(file);
	if ((*files)[files_length - 1] == NULL) {
		outOfMemory();
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
	
	MEMORY_FREE(&filepath);
	
    return error;
}

/* vfs_callback_loadMod
Loads a mod. *.cfg and *.lua are read into memory.
*/
int vfs_callback_loadGame(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int e = ERR_OK;

	if (!strcmp(command, "")) return ERR_OK;

	Allocator arena;
	e = allocator_create_stdlibArena(&arena);
	if (e) goto cleanup;

	size_t command_length = strlen(command);

	(void) str4_copyC(&g_game, (uint8_t *) command, command_length);
	e = g_game.error;
	if (e) goto cleanup;

	if (g_game.error) {
		e = g_game.error;
		goto cleanup;
	}

	// All mods will be mounted to the root directory.
	e = PHYSFS_mount(command, "", true);
	if (!e) {
		error("Could not add directory \"%s\" to the search path: %s",
		      command,
		      PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}
	else {
		info("Mounted \"%s\".", command);
	}
	e = ERR_OK;

	if (g_workspace.str_length == 0) {
		warning("Workspace is not set, so a directory for the current game will not be created in the workspace.", "");
	}
	else {
		Str4 slash = STR4("/");
		Str4 fullWorkspacePath = str4_create(&arena);
		(void) str4_append(&fullWorkspacePath, &g_workspace);
		(void) str4_append(&fullWorkspacePath, &slash);
		(void) str4_append(&fullWorkspacePath, &g_game);
		e = fullWorkspacePath.error;
		if (e) goto cleanup;
		e = PHYSFS_mount((char *)fullWorkspacePath.str, "", true);
		if (!e) {
			error("Could not add workspace directory \"%s\" to the search path: %s",
			      (char *)fullWorkspacePath.str,
			      PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		}
		else {
			info("Mounted \"%s\".", fullWorkspacePath.str);
		}
		e = ERR_OK;
		e = arena.quit(arena.context);
		if (e) goto cleanup;
	}

	// Make space for the next mod.
	
	g_mods.mods_length++;
	g_mods.mods = realloc(g_mods.mods, g_mods.mods_length * sizeof(vfs_mod_t));
	if (g_mods.mods == NULL) {
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	g_mods.mods[g_mods.mods_length - 1].files = NULL;
	g_mods.mods[g_mods.mods_length - 1].filenames = NULL;
	g_mods.mods[g_mods.mods_length - 1].files_length = 0;
	
	// Load files from mod directory.
	
	info("Searching for scripts in mod \"%s\"", command);
	
	e = PHYSFS_enumerate(command, vfs_PHYSFS_saveScriptFilesEnumerator, NULL);
	if (!e) {
		error("Could not enumerate files in directory \"%s\": %s",
		      command,
		      PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		e = ERR_CRITICAL;
		goto cleanup;
	}
	e = ERR_OK;

 cleanup: return e;
}


/* Config callbacks */
/* ================ */

int vfs_callback_setWorkspace(cfg2_var_t *var, const char *command, lua_State *luaState) {
	if (strlen(var->string) == 0) return ERR_OK;
	str4_copyC(&g_workspace, (uint8_t *) var->string, strlen(var->string));
	if ((g_workspace.str_length != 0) && !PHYSFS_setWriteDir((char *) g_workspace.str)) {
		error("Could not set workspace to \"%s\": %s", g_workspace, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return ERR_GENERIC;
	}
	return ERR_OK;
}

/* VFS helper functions */
/* ==================== */

int vfs_getFileText(char **fileText, const char *path) {
	int error = ERR_CRITICAL;
	
	PHYSFS_File *file = PHYSFS_openRead(path);
	if (file == NULL) {
		error("Could not open file \"%s\": %s", path, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	int fileText_length = PHYSFS_fileLength(file);
	
	error = str2_realloc(fileText, fileText_length);
	if (error) {
		goto cleanupPhysfs_l;
	}
	memset(*fileText, 0, (fileText_length + 1) * sizeof(char));
	
	PHYSFS_sint64 realLength = PHYSFS_readBytes(file, *fileText, fileText_length);
	(*fileText)[realLength] = '\0';
	
	error = ERR_OK;
	cleanupPhysfs_l:
	
	PHYSFS_close(file);
	
	cleanup_l:
	return error;
}

int vfs_getFileContents_malloc(uint8_t **fileContents, PHYSFS_sint64 *fileContents_length, const uint8_t *path) {
	int error = ERR_CRITICAL;

	// PhysFS uses UTF-8;
	const char *physPath = (const char *) path;
	PHYSFS_File *file = PHYSFS_openRead(physPath);
	if (file == NULL) {
		error("Could not open file \"%s\": %s", path, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	*fileContents_length = PHYSFS_fileLength(file);
	
	*fileContents = malloc((*fileContents_length + 1) * sizeof(uint8_t));
	if (*fileContents == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanupPhysfs_l;
	}
	
	PHYSFS_sint64 realLength = PHYSFS_readBytes(file, *fileContents, *fileContents_length);
	(*fileContents)[realLength] = '\0';  // Just in case it's text.
	*fileContents_length = realLength;
	
	error = ERR_OK;
	cleanupPhysfs_l:
	
	PHYSFS_close(file);
	
	cleanup_l:
	return error;
}
