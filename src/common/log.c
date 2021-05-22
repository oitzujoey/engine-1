
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "insane.h"

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
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
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
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
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
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
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
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
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
	int error = ERR_CRITICAL;

	const char *function = NULL;
	const char *message = NULL;
	char *buf = NULL;
	
	if (!lua_isstring(l, 1) || !lua_isstring(l, 2)) {
		error("Arguments must be strings", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	function = lua_tostring(l, 1);
	message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Info: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	buf = malloc((strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, message);
	
	printf(buf);
	
	error = ERR_OK;
	cleanup_l:
	
	insane_free(buf);
	
	if (error) {
		lua_error(l);
	}
	
	return 0;
}

int l_log_warning(lua_State *l) {
	const char *function = lua_tostring(l, 1);
	const char *message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_MAGENTA"Warning: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, buf);
	
	free(buf);
	
	return 0;
}

int l_log_error(lua_State *l) {
	const char *function = lua_tostring(l, 1);
	const char *message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, buf);
	
	free(buf);
	
	return 0;
}

int l_log_critical_error(lua_State *l) {
	const char *function = lua_tostring(l, 1);
	const char *message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Critical error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, message);
	
	fprintf(stderr, buf);
	
	free(buf);
	
	return 0;
}
