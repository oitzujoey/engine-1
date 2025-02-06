
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

/*
0   info
1   warning
2   error
3   critical error
4   nothing
*/
int g_logLevel = 0;

int log_callback_updateLogLevel(cfg2_var_t *var, const char *command, lua_State *luaState) {
	if (var->integer > 4) {
		var->integer = 4;
		warning("Value out of range. Setting to %i.", var->integer);
	}
	else if (var->integer < 0) {
		var->integer = 0;
		warning("Value out of range. Setting to %i.", var->integer);
	}
	
	g_logLevel = var->integer;
	
	return 0;
}

void log_info(const char *function, const char *fmt, ...) {
	va_list va;
	
	if (g_logLevel > 0) {
		return;
	}
	
	const char *infomessage = COLOR_GREEN"Info: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("log_info");
		return;
	}
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vprintf(buf, va);
	va_end(va);

	free(buf);
}

void log_warning(const char *function, const char *fmt, ...) {
	va_list va;
	
	if (g_logLevel > 1) {
		return;
	}
	
	const char *infomessage = COLOR_MAGENTA"Warning: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("log_warning");
		return;
	}
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

void log_error(const char *function, const char *fmt, ...) {
	va_list va;
	
	if (g_logLevel > 2) {
		return;
	}
	
	const char *infomessage = COLOR_YELLOW"Error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("log_error");
		return;
	}
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

void log_critical_error(const char *function, const char *fmt, ...) {
	va_list va;
	
	if (g_logLevel > 3) {
		return;
	}
	
	const char *infomessage = COLOR_RED"Critical error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("log_critical_error");
		return;
	}
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

void log_outOfMemory(const char *function) {

	size_t length;
	const char *infoMessageStart = COLOR_RED"Critical error: "COLOR_BLUE"(";
	const char *infoMessageEnd = ")"COLOR_NORMAL" Out of memory.\n";
	
	length = strlen(infoMessageStart);
	for (unsigned int i = 0; i < length; i++) {
		putc(infoMessageStart[i], stderr);
	}
	
	length = strlen(function);
	for (unsigned int i = 0; i < length; i++) {
		putc(function[i], stderr);
	}
	
	length = strlen(infoMessageEnd);
	for (unsigned int i = 0; i < length; i++) {
		putc(infoMessageEnd[i], stderr);
	}
}

int l_log_info(lua_State *l) {
	int e = ERR_OK;

	const char *function = NULL;
	const char *message = NULL;
	char *buf = NULL;
	
	if (!lua_isstring(l, 1) || !lua_isstring(l, 2)) {
		error("Arguments must be strings", "");
		e = ERR_GENERIC;
		goto cleanup;
	}
	
	function = lua_tostring(l, 1);
	message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Info: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1;
	buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("l_log_info");
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	sprintf(buf, infomessage, function, message);
	
	printf("%s", buf);

	free(buf);
 cleanup:
	if (e) lua_error(l);
	return 0;
}

int l_log_warning(lua_State *l) {
	int e = ERR_OK;

	const char *function = NULL;
	const char *message = NULL;

	if (!lua_isstring(l, 1) || !lua_isstring(l, 2)) {
		error("Arguments must be strings", "");
		e = ERR_GENERIC;
		goto cleanup;
	}

	function = lua_tostring(l, 1);
	message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_MAGENTA"Warning: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("l_log_warning");
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, "%s", buf);

	free(buf);
 cleanup:
	if (e) lua_error(l);
	return 0;
}

int l_log_error(lua_State *l) {
	int e = ERR_OK;

	const char *function = NULL;
	const char *message = NULL;

	if (!lua_isstring(l, 1) || !lua_isstring(l, 2)) {
		error("Arguments must be strings", "");
		e = ERR_GENERIC;
		goto cleanup;
	}

	function = lua_tostring(l, 1);
	message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_YELLOW"Error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("l_log_error");
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, "%s", buf);
	
	free(buf);
 cleanup:
	if (e) lua_error(l);
	return 0;
}

int l_log_critical_error(lua_State *l) {
	int e = ERR_OK;

	const char *function = NULL;
	const char *message = NULL;

	if (!lua_isstring(l, 1) || !lua_isstring(l, 2)) {
		error("Arguments must be strings", "");
		e = ERR_GENERIC;
		goto cleanup;
	}

	function = lua_tostring(l, 1);
	message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Critical error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	size_t buf_length = strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1;
	char *buf = malloc(buf_length * sizeof(char));
	if (!buf && buf_length) {
		log_outOfMemory("l_log_critical_error");
		e = ERR_OUTOFMEMORY;
		goto cleanup;
	}
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, "%s", buf);
	
	free(buf);
 cleanup:
	if (e) lua_error(l);	
	return 0;
}
