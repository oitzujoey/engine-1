
#ifndef SNETWORK_H
#define SNETWORK_H

// #include <SDL2/SDL.h>
// #include <SDL2/SDL_net.h>
#include <enet/enet.h>
#include "../common/cfg.h"

// typedef struct {
// 	int in_use;
// 	UDPsocket socket;
// } client_t;

// extern client_t g_clients[MAX_CLIENTS];
// extern UDPsocket g_serverSocket;

// int l_snetwork_send(const uint8_t *data, int length, IPaddress ipAddress);

int snetwork_handle_setServerPort(cfg_var_t *var);
int snetwork_handle_enetMessage(cfg_var_t *var);
int snetwork_handle_maxClients(cfg_var_t *var);

int snetwork_runEvents(void);
int snetwork_init(void);
void snetwork_quit(void);

#endif
