
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_net.h>
#include <enet/enet.h>
#include "cfg.h"

#define ENET_CHANNELS       2

// typedef struct {
// 	uint16_t id;
// 	uint16_t idAck;
// 	uint16_t flags;
	
// } reliablePacket_t;

// #define MAX_PACKET_LENGTH    100

// int network_sendReliablePacket(UDPsocket socket, IPaddress ipAddress, uint8_t *data, int length);
// int network_receiveReliablePacket(UDPsocket socket, uint8_t **data, int *length);

extern int g_connectionTimeout;

int network_ipv4ToString(char **string, uint32_t ipAddress);

int network_handle_connectionTimeout(cfg_var_t *var);

#endif
