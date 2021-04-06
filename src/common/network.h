
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_net.h>
#include <enet/enet.h>
#include "cfg2.h"

#define ENET_CHANNELS       2
#define ENET_CHANNEL0       0
#define ENET_CHANNEL1       1

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

int network_callback_connectionTimeout(cfg2_var_t *var, const char *command, lua_State *luaState);

uint32_t network_generateChecksum(uint8_t *data, size_t length);

#endif
