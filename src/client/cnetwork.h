
#ifndef CNETWORK_H
#define CNETWORK_H

#include <stdbool.h>
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_net.h>
#include <enet/enet.h>
#include "../common/cfg.h"

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

/* Config events */
int cnetwork_handle_setIpAddress(cfg_var_t *var);
int cnetwork_handle_setServerPort(cfg_var_t *var);
int cnetwork_handle_setClientPort(cfg_var_t *var);

/* Network functions */
int cnetwork_runEvents(void);
int cnetwork_init(void);
void cnetwork_quit(void);

#endif
