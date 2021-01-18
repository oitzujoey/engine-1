
#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <lua.h>
#include "types.h"

#define ERR_OK          0
#define ERR_GENERIC     1
#define ERR_CRITICAL    2
#define ERR_OUTOFMEMORY 3

// @TODO: Might want to change this to an actual variable.
#define ERR ((char*[15]) { \
	"OK", \
	"Generic error", \
	"Critical error", \
	"Out of memory" \
})

#define COLOR_NORMAL    "\x1B[0m"
#define COLOR_BLACK     "\x1B[30m"
#define COLOR_RED       "\x1B[31m"
#define COLOR_GREEN     "\x1B[32m"
#define COLOR_YELLOW    "\x1B[33m"
#define COLOR_BLUE      "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN      "\x1B[36m"
#define COLOR_WHITE     "\x1B[37m"

#define B_COLOR_BLACK     "\x1B[40m"
#define B_COLOR_RED       "\x1B[41m"
#define B_COLOR_GREEN     "\x1B[42m"
#define B_COLOR_YELLOW    "\x1B[43m"
#define B_COLOR_BLUE      "\x1B[44m"
#define B_COLOR_MAGENTA   "\x1B[45m"
#define B_COLOR_CYAN      "\x1B[46m"
#define B_COLOR_WHITE     "\x1B[47m"

#define AUTOEXEC    "autoexec.cfg"

#define MAX_CLIENTS         2

#define ENGINE_MAN_NAME "Scott"

extern int error;

int common_getTimeNs(long *ns);

int l_puts(lua_State*);

int MSB(int n);

#endif
