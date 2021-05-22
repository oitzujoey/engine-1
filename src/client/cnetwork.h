
#ifndef CNETWORK_H
#define CNETWORK_H

#include "../common/types.h"

#define CNETWORK_STATESTRING_TYPE_STRING    0U

#define CNETWORK_STATESTRING_TYPE_LENGTH    1UL
#define CNETWORK_STATESTRING_LENGTH_LENGTH  2UL

// extern UDPsocket g_clientSocket;

// int cnetwork_closeSocket(UDPsocket socket);
// int l_cnetwork_receive(Uint8 **data, int *length);

typedef enum cnetwork_connectionState_e {
	cnetwork_connectionState_initial,
	cnetwork_connectionState_connecting,
	cnetwork_connectionState_connected,
	cnetwork_connectionState_disconnected
} cnetwork_connectionState_t;

typedef struct cnetwork_clientState_s {
	bool connected;
	ENetAddress serverAddress;
	const char *serverAddressString;
	cnetwork_connectionState_t connectionState;
	time_t lastResponseTime;
} cnetwork_clientState_t;

/* Config callbacks */
int cnetwork_callback_setIpAddress(cfg2_var_t *var, const char *command, lua_State *luaState);
int cnetwork_callback_setServerPort(cfg2_var_t *var, const char *command, lua_State *luaState);

/* Lua functions */

/* Network functions */
int cnetwork_sendString(const char *string);
int cnetwork_runEvents(lua_State *luaState);
int cnetwork_init(void);
void cnetwork_quit(void);

#endif
