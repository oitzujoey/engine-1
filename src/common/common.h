
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

#define DEFAULT_PORT_NUMBER 8099

#define CFG_MAX_RECURSION   "max_recursion_depth"
#define CFG_MAX_RECURSION_DEFAULT   10
#define CFG_RUN_QUIET       "quiet"
#define CFG_HISTORY_LENGTH  "command_history_length"
#define CFG_HISTORY_LENGTH_DEFAULT  10

#define ENGINE_MAN_NAME "Scott"

#ifdef DOUBLE_VEC
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

typedef struct {
	vec_t s;
	vec3_t v;
} quat_t;

extern int error;

extern vfs_t g_vfs;

int l_puts(lua_State*);

int MSB(int n);

void vec3_copy(vec3_t *destination, vec3_t *source);
void vec3_dotProduct(vec_t *result, vec3_t *a, vec3_t *b);
void vec3_crossProduct(vec3_t *result, vec3_t *a, vec3_t *b);
void vec3_subtract(vec3_t *result, vec3_t *a, vec3_t *b);
int vec3_normalize(vec3_t *v);

#endif
