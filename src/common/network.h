
#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"

#define ENET_CHANNELS       2
#define ENET_CHANNEL0       0
#define ENET_CHANNEL1       1

#define NETWORK_LUA_CLIENTSTATE_NAME    "clientState"
#define NETWORK_LUA_SERVERSTATE_NAME    "serverState"
#define NETWORK_LUA_MAXCLIENTS_NAME     "maxClients"

// typedef struct {
// 	uint16_t id;
// 	uint16_t idAck;
// 	uint16_t flags;
	
// } reliablePacket_t;

// #define MAX_PACKET_LENGTH    100

// int network_sendReliablePacket(UDPsocket socket, IPaddress ipAddress, uint8_t *data, int length);
// int network_receiveReliablePacket(UDPsocket socket, uint8_t **data, int *length);

extern int g_connectionTimeout;

int network_packetAdd_uint32(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const uint32_t *data, const ptrdiff_t data_length);
int network_packetAdd_ptrdiff(enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length, const ptrdiff_t *data, const ptrdiff_t data_length);

int network_packetRead_uint32(uint32_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length);
int network_packetRead_entityList(entityList_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length);
int network_packetRead_entity(entity_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length);
int network_packetRead_ptrdiff(ptrdiff_t *data, const ptrdiff_t data_length, const enet_uint8 *packet, ptrdiff_t *index, const ptrdiff_t packet_length);


int network_packetAdd_lua_object(lua_State *luaState, const void *name, network_lua_type_t nameType, ENetPacket *packet, ptrdiff_t *index);
int network_packetRead_lua_object(lua_State *luaState, ENetPacket *packet, ptrdiff_t *index);

int network_ipv4ToString(char **string, uint32_t ipAddress);

int network_callback_connectionTimeout(cfg2_var_t *var, const char *command, lua_State *luaState);

uint32_t network_generateChecksum(const uint8_t *data, size_t length);

#endif
