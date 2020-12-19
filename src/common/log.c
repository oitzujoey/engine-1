
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

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
	
	const char *infomessage = COLOR_YELLOW"Warning: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

void log_error(const char *function, const char *fmt, ...) {
	va_list va;
	
	const char *infomessage = COLOR_RED"Error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}

void log_critical_error(const char *function, const char *fmt, ...) {
	va_list va;
	
	const char *infomessage = COLOR_MAGENTA"Critical error: "COLOR_BLUE"(%s)"COLOR_NORMAL" %s\n";
	char *buf = malloc((strlen(infomessage) + strlen(function) + strlen(fmt) - 4 + 1) * sizeof(char));
	sprintf(buf, infomessage, function, fmt);
	
	va_start(va, fmt);
	vfprintf(stderr, buf, va);
	va_end(va);
	
	free(buf);
}
