
#ifndef SNETWORK_H
#define SNETWORK_H

// #include <SDL2/SDL.h>
// #include <SDL2/SDL_net.h>
#include <enet/enet.h>
#include "../common/cfg2.h"

// typedef struct {
// 	int in_use;
// 	UDPsocket socket;
// } client_t;

// extern client_t g_clients[MAX_CLIENTS];
// extern UDPsocket g_serverSocket;

// int l_snetwork_send(const uint8_t *data, int length, IPaddress ipAddress);

int snetwork_callback_setServerPort(cfg2_var_t *var, const char *command);
int snetwork_callback_enetMessage(cfg2_var_t *var, const char *command);
int snetwork_callback_maxClients(cfg2_var_t *var, const char *command);

int snetwork_runEvents(void);
int snetwork_init(void);
void snetwork_quit(void);

#endif
