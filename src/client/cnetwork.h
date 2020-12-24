
#ifndef CNETWORK_H
#define CNETWORK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

int cnetwork_closeSocket(UDPsocket socket);
int l_cnetwork_receive(Uint8 *data, int *length);

int cnetwork_init(const char *serverAddress);
void cnetwork_quit(void);

#endif
