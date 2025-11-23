#include <lua.h>
#include "../common/str4.h"

#ifdef LINUX

// Returns ERR_GENERIC if it was unable to create and open the file.
int namedPipe_init(void);
void namedPipe_quit(void);

// namedPipe_readAsString -> string::(String|Nil) e::Integer
int l_namedPipe_readAsString(lua_State *l);

#endif
