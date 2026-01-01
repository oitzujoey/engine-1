
#ifndef SNETWORK_H
#define SNETWORK_H

#include "../common/types.h"

// typedef struct {
// 	int in_use;
// 	UDPsocket socket;
// } client_t;

// extern client_t g_clients[MAX_CLIENTS];
// extern UDPsocket g_serverSocket;

int snetwork_callback_setIpAddress(cfg2_var_t *var, const char *command, lua_State *luaState);
int snetwork_callback_setServerPort(cfg2_var_t *var, const char *command, lua_State *luaState);
int snetwork_callback_maxClients(cfg2_var_t *var, const char *command, lua_State *luaState);

int l_snetwork_disconnect(lua_State *l);

int snetwork_runEvents(lua_State *luaState);
int snetwork_init(void);
void snetwork_quit(void);

#endif
