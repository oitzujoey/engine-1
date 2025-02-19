#include "lua_sqlite.h"
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "../sqlite/sqlite3.h"
#include "vfs.h"
#include "str4.h"
#include "arena.h"

int l_sqlite_open(lua_State *l) {
	int e = ERR_OK;

	int argc = lua_gettop(l);
	if (argc != 1) {
		error("Requires 1 argument", "");
		lua_error(l);
	}
	if (!lua_isstring(l, 1)) {
		error("Path to database must be a string.", "");
		lua_error(l);
	}
	const char *dbName_c = lua_tostring(l, 1);

	Allocator stringArena;
	e = allocator_create_stdlibArena(&stringArena);
	if (e) {
		lua_error(l);
		goto cleanup;
	}
	Str4 path = str4_create(&stringArena);
	Str4 dbName = str4_createConstant((uint8_t *) dbName_c, strlen(dbName_c));
	Str4 slash = STR4("/");
	Str4 extension = STR4(".db");
	(void) str4_append(&path, &g_workspace);
	(void) str4_append(&path, &slash);
	(void) str4_append(&path, &g_game);
	(void) str4_append(&path, &slash);
	(void) str4_append(&path, &dbName);
	(void) str4_append(&path, &extension);
	e = str4_errorp(&path);
	if (e) {
		error("Failed to build file system path to database.", "");
		lua_error(l);
		goto cleanup;
	}

	sqlite3 *db = NULL;
	int sqlite_e = sqlite3_open((char *) path.str, &db);
	if (sqlite_e != SQLITE_OK) {
		error("Failed to open database \"%s\". SQLite error: \"%s\"", path.str, sqlite3_errmsg(db));
		e = ERR_GENERIC;
		goto cleanup;
	}
	info("Opened database \"%s\".", dbName.str);

	sqlite3 **dbContainer = lua_newuserdata(l, sizeof(sqlite3 *));
	*dbContainer = db;

	e = stringArena.quit(stringArena.context);
	if (e) goto cleanup;

 cleanup: return 1;
}


int lua_sqlite_callback(void *firstArgument, int numberOfColumns, char **columnStrings, char **resultColumnNameStrings) {
	lua_State *l = firstArgument;
	printf("lua_sqlite_callback: %i\n", numberOfColumns);
	return SQLITE_OK;
}

int l_sqlite_exec(lua_State *l) {
	int e = ERR_OK;

	int argc = lua_gettop(l);
	if (argc != 2) {
		error("Requires 2 arguments: database, query", "");
		lua_error(l);
	}
	if (!lua_isuserdata(l, 1)) {
		error("Database must be a database object.", "");
		lua_error(l);
	}
	if (!lua_isstring(l, 2)) {
		error("SQL query must be a string.", "");
		lua_error(l);
	}
	sqlite3 **dbContainer = lua_touserdata(l, 1);
	const char *query_c = lua_tostring(l, 2);
	char *errorMessage = NULL;

	int sqlite_e = sqlite3_exec(*dbContainer, query_c, NULL, NULL, &errorMessage);
	if (sqlite_e != SQLITE_OK){
		error("Failed to execute query \"%s\". SQLite error code: \"%s\". SQLite error message: \"%s\"",
		      query_c,
		      sqlite3_errmsg(*dbContainer),
		      errorMessage);
		if (errorMessage != NULL) sqlite3_free(errorMessage);
		goto cleanup;
	}

 cleanup:
	if (e) lua_error(l);
	(void) lua_pushinteger(l, sqlite_e);
	return 1;
}

int l_sqlite_close(lua_State *l) {
	int e = ERR_OK;

	int argc = lua_gettop(l);
	if (argc != 1) {
		error("Requires 1 argument: database", "");
		lua_error(l);
	}
	if (!lua_isuserdata(l, 1)) {
		error("Database must be a database object.", "");
		lua_error(l);
	}
	sqlite3 **dbContainer = lua_touserdata(l, 1);

	int sqlite_e = sqlite3_close(*dbContainer);
	if (sqlite_e != SQLITE_OK){
		error("Failed to close database. SQLite error code: \"%s\"", sqlite3_errmsg(*dbContainer));
		e = ERR_GENERIC;
		goto cleanup;
	}

 cleanup:
	(void) lua_pushnil(l);
	return 1;
}
