
#ifndef SNETWORK_H
#define SNETWORK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#define MAX_CLIENTS 2

typedef struct {
	int in_use;
	UDPsocket socket;
} client_t;

extern client_t clients_g[MAX_CLIENTS];

int l_snetwork_send(const uint8_t *data, int length, IPaddress ipAddress);
int snetwork_init(void);
void snetwork_quit(void);

#endif
