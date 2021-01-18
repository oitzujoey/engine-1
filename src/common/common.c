
#include "common.h"
#include <stdio.h>
#include <math.h>

int error;


int common_getTimeNs(long *ns) {
	int error = ERR_CRITICAL;
	
// #ifdef LINUX
// 	struct timespec timespec;
	
// 	error = clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
// 	if (error < 0) {
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
// 	*ns = timespec.tv_nsec;
// #else
// #error "common_getTimeNs has not been rewritten for this platform."
// #endif
	error = ERR_OK;
	// cleanup_l:
	return error;
}

int l_puts(lua_State *Lua) {
    printf(COLOR_CYAN"Lua {"COLOR_NORMAL"%s"COLOR_CYAN"}"COLOR_NORMAL"\n", lua_tostring(Lua, 1));
    return 0;
}

int MSB(int n) {
	int i;
	
	for (i = 0; (1<<i) <= n; i++);
	
	return i - 1;
}
