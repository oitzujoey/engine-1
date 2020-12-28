
#ifndef COMMON_H
#define COMMON_H

#include <lua.h>
#include "vfs.h"

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
#define COLOR_RED       "\x1B[31m"
#define COLOR_GREEN     "\x1B[32m"
#define COLOR_YELLOW    "\x1B[33m"
#define COLOR_BLUE      "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN      "\x1B[36m"
#define COLOR_WHITE     "\x1B[37m"

#define AUTOEXEC    "autoexec.cfg"

#define DEFAULT_PORT_NUMBER 8099

#ifdef DOUBLE_VEC
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

extern int error;

extern vfs_t vfs_g;

int l_puts(lua_State*);

int MSB(int n);

#endif
