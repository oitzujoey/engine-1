
#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <zip.h>
#include "common.h"
#include "file.h"
#include "log.h"

int vfs_getFileText(vfs_t *vfs, string_t *fileText, const string_t *workspace_path) {
	
	if (vfs->workspace_type == vfs_type_directory) {
		string_t path;
		string_init(&path);
		string_concatenate(&path, &vfs->path);
		string_concatenate_c(&path, "/");
		string_concatenate(&path, workspace_path);
		
		free(fileText->value);
		fileText->value = file_getText(path.value);
		string_normalize(fileText);

		string_free(&path);
		
		return ERR_OK;
	}
	return ERR_GENERIC;
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

int vfs_free(vfs_t *vfs) {
	string_free(&vfs->path);
}
