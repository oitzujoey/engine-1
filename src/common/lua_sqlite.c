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
		sqlite3_close(db);
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


/* int lua_sqlite_callback(void *firstArgument, int numberOfColumns, char **columnStrings, char **resultColumnNameStrings) { */
/* 	lua_State *l = firstArgument; */
/* 	for (int column_index = 0; column_index < numberOfColumns; column_index++) { */
/* 		printf("%s: %s", resultColumnNameStrings[column_index], columnStrings[column_index]); */
/* 		if (column_index == numberOfColumns-1) { */
/* 			putchar('\n'); */
/* 		} */
/* 		else { */
/* 			printf(", "); */
/* 		} */
/* 	} */
/* 	return SQLITE_OK; */
/* } */

/* l_sqlite_exec(database, query) -> result, error
   Makes a query to the database and returns the result of the query as a table. */
int l_sqlite_exec(lua_State *l) {
	int e = ERR_OK;

	int stackTop = lua_gettop(l);
	{
		int argc = stackTop;
		if (argc != 2) {
			error("Requires 2 arguments: database, query", "");
			lua_error(l);
		}
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
	size_t query_c_length;
	const char *query_c = lua_tolstring(l, 2, &query_c_length);

	if (!lua_checkstack(l, 5)) {
		error("Not enough room on Lua's stack to push the query result.", "");
		e = ERR_GENERIC;
		goto cleanup;
	}

	sqlite3_stmt *statement;
	int sqlite_e = sqlite3_prepare_v2(*dbContainer, query_c, query_c_length, &statement, NULL);
	if (sqlite_e != SQLITE_OK) {
		error("Failed to prepare query \"%s\". SQLite error code: \"%s\"", query_c, sqlite3_errmsg(*dbContainer));
		goto cleanup;
	}

	(void) lua_newtable(l);
	// stack: {}

	int row_index = 0;
	while (true) {
		sqlite_e = sqlite3_step(statement);
		if (sqlite_e == SQLITE_DONE) {
			sqlite_e = SQLITE_OK;
			break;
		}
		else if (sqlite_e == SQLITE_ROW) {
			(void) lua_pushinteger(l, row_index + 1);
			// stack: result row_index
			(void) lua_newtable(l);
			// stack: result row_index row

			int column_count = sqlite3_column_count(statement);
			for (int column_index = 0; column_index < column_count; column_index++) {
				const char *column_name = sqlite3_column_name(statement, column_index);

				// stack: result row_index row
				(void) lua_pushstring(l, column_name);
				// stack: result row_index row column_index

				switch (sqlite3_column_type(statement, column_index)) {
				case SQLITE_INTEGER:{
					// SQLite returns `long long` for _int64, at least on my platform.
					const long long value = sqlite3_column_int64(statement, column_index);
					// Lua also uses long long for integers, at least on my platform.
					(void) lua_pushinteger(l, value);
					// stack: result row_index row column_index integer
					break;
				}
				case SQLITE_FLOAT:{
					const double value = sqlite3_column_double(statement, column_index);
					// Lua uses `double` for non-integer numbers, at least on my platform.
					(void) lua_pushnumber(l, value);
					// stack: result row_index row column_index double
					break;
				}
				case SQLITE_TEXT: {
					const uint8_t *value = sqlite3_column_text(statement, column_index);
					(void) lua_pushstring(l, (const char *) value);
					// stack: result row_index row column_index string
					break;
				}
				case SQLITE_BLOB:
					(void) lua_pushnil(l);
					// stack: result row_index row column_index nil
					break;
				case SQLITE_NULL:{
					(void) lua_pushnil(l);
					// stack: result row_index row column_index nil
					break;
				}
				}
				// stack: result row_index row column_index value
				(void) lua_settable(l, -3);
				// stack: result row_index row
			}
			// stack: result row_index row
			(void) lua_settable(l, -3);
			// stack: result
		}
		else {
			error("Failed to get result of query \"%s\". SQLite error message: \"%s\"",
			      query_c,
			      sqlite3_errmsg(*dbContainer));
			break;
		}
		row_index++;
	}
	if (sqlite_e != SQLITE_OK) goto cleanup;

 cleanup:
	if (statement != NULL) (void) sqlite3_finalize(statement);
	if (sqlite_e) {
		// Remove any junk we pushed on the stack that still remains.
		(void) lua_settop(l, stackTop);
		// Push an empty table as the result.
		(void) lua_newtable(l);
	}
	if (e) lua_error(l);
	(void) lua_pushinteger(l, sqlite_e);
	return 2;
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
