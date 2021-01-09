
#include "network.h"
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "log.h"

int g_connectionTimeout;

// int network_sendReliablePacket(UDPsocket socket, IPaddress ipAddress, uint8_t *data, int length) {
// 	int error = ERR_CRITICAL;
	
// 	static uint16_t id = 0;
// 	reliablePacket_t reliablePacket;
// 	Uint8 *dataStart;
	
// 	reliablePacket.id = id++;
// 	reliablePacket.idAck = 0;
// 	reliablePacket.flags = 0;
	
// 	// Allocate packet.
// 	UDPpacket *packet = SDLNet_AllocPacket((length * sizeof(uint8_t) + sizeof(reliablePacket_t)));
// 	if (packet == NULL) {
// 		error = ERR_OUTOFMEMORY;
// 		goto cleanup_l;
// 	}
	
// 	packet->address = ipAddress;
// 	packet->len = length * sizeof(uint8_t) + sizeof(reliablePacket_t);
	
// 	// Construct header.
// 	dataStart = packet->data;
	
// 	// Little endian.
// 	*(dataStart++) = reliablePacket.id & 0x00FF;
// 	*(dataStart++) = (reliablePacket.id & 0xFF00) >> 8;
// 	*(dataStart++) = reliablePacket.idAck & 0x00FF;
// 	*(dataStart++) = (reliablePacket.idAck & 0xFF00) >> 8;
// 	*(dataStart++) = reliablePacket.flags & 0x00FF;
// 	*(dataStart++) = (reliablePacket.flags & 0xFF00) >> 8;
	
// 	// Copy data.
// 	memcpy(dataStart, data, length * sizeof(Uint8));

// 	error = SDLNet_UDP_Send(socket, -1, packet);
// 	if (error == 0) {
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	error = ERR_OK;
// 	cleanup_l:
	
// 	SDLNet_FreePacket(packet);
	
// 	return error;
// }

// int network_receiveReliablePacket(UDPsocket socket, uint8_t **data, int *length) {
// 	int error = ERR_CRITICAL;
	
// 	reliablePacket_t reliablePacket;
// 	Uint8 *dataPointer;
// 	static uint16_t idCheck = 0;
// 	static int init = 0;
	
// 	UDPpacket *packet = SDLNet_AllocPacket(MAX_PACKET_LENGTH);
// 	if (packet == NULL) {
// 		error = ERR_OUTOFMEMORY;
// 		goto cleanup_l;
// 	}
	
// 	error = SDLNet_UDP_Recv(socket, packet);
// 	if (error < 0) {
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
// 	// We received something.
// 	if (error > 0) {
// 		*length = packet->len - sizeof(reliablePacket_t);
// 		*data = malloc(*length * sizeof(Uint8));
// 		if (*data == NULL) {
// 			error = ERR_OUTOFMEMORY;
// 			goto cleanup_l;
// 		}
		
// 		dataPointer = packet->data;
// 		// Little endian
// 		reliablePacket.id = *(dataPointer++);
// 		reliablePacket.id |= *(dataPointer++)<<8;
// 		reliablePacket.idAck = *(dataPointer++);
// 		reliablePacket.idAck |= *(dataPointer++)<<8;
// 		reliablePacket.flags = *(dataPointer++);
// 		reliablePacket.flags |= *(dataPointer++)<<8;
		
// 		// Check for out of order packets.
// 		if (idCheck > reliablePacket.id) {
// 			printf("Note: Late packet %u\n", reliablePacket.id);
// 			error = ERR_OK;
// 			goto cleanup_l;
// 		}
// 		// Check for missed packets.
// 		if (init == 0) {
// 			idCheck = reliablePacket.id;
// 			init++;
// 		}
// 		else for (; idCheck < reliablePacket.id; idCheck++) {
// 			printf("Note: Missed packet %u\n", idCheck);
// 		}
		
// 		memcpy(*data, dataPointer, *length);
		
// 		// printf(
// 		// 	"ID: %i\n"
// 		// 	"ID ACK: %i\n"
// 		// 	"Flags: %X\n"
// 		// 	"String: %s\n"
// 		// 	"\n",
// 		// 	reliablePacket.id,
// 		// 	reliablePacket.idAck,
// 		// 	reliablePacket.flags,
// 		// 	*data
// 		// );
		
// 		// Prepare for next packet.
// 		idCheck++;
// 	}
// 	else {
// 		*length = 0;
// 	}
	
// 	error = ERR_OK;
// 	cleanup_l:
	
// 	SDLNet_FreePacket(packet);
	
// 	return error;
// }

int network_ipv4ToString(char **string, uint32_t ipAddress) {
	int error = ERR_CRITICAL;

	uint8_t byte[4];
	
	*string = realloc(*string, (strlen("255.255.255.255") + 1) * sizeof(char));
	if (*string == NULL) {
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	for (int i = 0; i < 4; i++) {
		byte[i] = (ipAddress >> (i * 8)) & 0x000000FF;
	}
	
	sprintf(*string, "%d.%d.%d.%d", byte[0], byte[1], byte[2], byte[3]);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int network_handle_connectionTimeout(cfg_var_t *var) {
	
	g_connectionTimeout = var->integer;
	
	if (var->integer < 1) {
		warning("Variable \"%s\" out of range. Setting to 1.", var->name);
		var->integer = 1;
	}
	
	return ERR_OK;
}
