#include "named_pipe.h"
#ifdef LINUX
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../common/log.h"
#include "../common/common.h"
#include "../common/arena.h"

int namedPipe_fd = -1;
int failure = 0;
const char *fifo_path = "engine1_client.pipe";


int namedPipe_init(void) {
	int mkfifo_result = mkfifo(fifo_path, 0660);
	if (mkfifo_result < 0) {
		error("Couldn't create named pipe. Maybe it already exists. Maybe it doesn't. Don't have time for this.", "");
		return ERR_GENERIC;
	}
	info("Created named pipe.", "");
	return ERR_OK;
}

void namedPipe_quit(void) {
	if (namedPipe_fd >= 0) {
		close(namedPipe_fd);
	}
}

// The caller is expected to free `string` using the arena.
// Returns ERR_OUTOFMEMORY and ERR_GENERIC. ERR_GENERIC is returned if the pipe was not opened.
Str4 namedPipe_readAsString(Allocator *arena) {
	static uint8_t read_buffer[1000];
	Str4 string = str4_create(arena);

	if (failure) {
		string.error = ERR_GENERIC;
		return string;
	}
	if (namedPipe_fd < 0) {
		namedPipe_fd = open(fifo_path, O_RDONLY);
		if (namedPipe_fd < 0) {
			error("Could not open named pipe.", "");
			failure = 1;
			string.error = ERR_GENERIC;
			return string;
		}
		info("Opened named pipe.", "");
	}

	ssize_t read_length;
	while (0 != (read_length = read(namedPipe_fd,
									(char *) read_buffer,
									sizeof(read_buffer)/sizeof(*read_buffer)))) {
		(void) str4_appendC(&string, read_buffer, read_length);
	}
	return string;
}

// namedPipe_readAsString -> string::(String|Nil) e::Integer
int l_namedPipe_readAsString(lua_State *l) {
	int e = ERR_OK;

	if (lua_gettop(l) != 0) {
		critical_error("Lua function %s doesn't accept arguments.", __func__);
		e = ERR_CRITICAL;
		goto cleanup;
	}

	Allocator arena;
	e = allocator_create_stdlibArena(&arena);
	if (e) {
		e = ERR_OUTOFMEMORY;
		(void) outOfMemory();
		goto cleanup;
	}

	Str4 string = namedPipe_readAsString(&arena);
	if (str4_errorp(&string)) {
		e = string.error;
	}
	if (e) {
		(void) lua_pushnil(l);
	}
	else {
		(void) lua_pushlstring(l, (const char *) string.str, string.str_length);
		Allocator *allocator = string.allocator;
		e = allocator->quit(allocator->context);
	}

 cleanup:
	if (e >= ERR_CRITICAL) lua_error(l);
	lua_pushinteger(l, e);
	return 2;
}

#endif
