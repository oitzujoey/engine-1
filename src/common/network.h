
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

typedef struct {
	uint16_t id;
	uint16_t idAck;
	uint16_t flags;
	
} reliablePacket_t;

#define MAX_PACKET_LENGTH    100

int network_sendReliablePacket(UDPsocket socket, IPaddress ipAddress, uint8_t *data, int length);
int network_receiveReliablePacket(UDPsocket socket, uint8_t **data, int *length);

#endif
