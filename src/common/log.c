
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_info(const char *function, const char *fmt, ...) {
	va_list va;
	
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
	
	const char *infomessage = COLOR_RED"Critical error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

int l_log_info(lua_State *l) {
	const char *function = lua_tostring(l, 1);
	const char *message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Info: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(message) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, message);
	
	printf(buf);
	
	free(buf);
	
	return 0;
}

int l_log_warning(lua_State *l) {
	const char *function = lua_tostring(l, 1);
	const char *message = lua_tostring(l, 2);

	const char *infomessage = COLOR_CYAN"Lua "COLOR_GREEN"Warning: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
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
