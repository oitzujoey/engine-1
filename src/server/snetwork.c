
#include "snetwork.h"
#include <stdio.h>
#include <string.h>
#include "../common/log.h"
#include "../common/insane.h"
#include "../common/network.h"
#include "server.h"
#include "../common/entity.h"
#include "../common/vector.h"
#include "../common/lua_common.h"

// client_t g_clients[MAX_CLIENTS];
// UDPsocket g_serverSocket;
// // SDLNet_SocketSet g_socketSet;

serverState_t g_server;
client_t g_clients[MAX_CLIENTS];


int snetwork_callback_setServerPort(cfg2_var_t *var, const char *command, lua_State *luaState) {

	// Shouldn't have to check type because it will always be hard coded when this function is run.
	if (var->integer < 1024) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, CFG_PORT_DEFAULT);
		var->integer = CFG_PORT_DEFAULT;
	}
	g_server.ipAddress.port = var->integer;

	return ERR_OK;
}

int snetwork_callback_enetMessage(cfg2_var_t *var, const char *command, lua_State *luaState) {

	if (strlen(var->string) == 0) {
		goto cleanup_l; 
	}
	
	ENetPacket *packet = enet_packet_create(var->string, strlen(var->string) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(g_server.host, ENET_CHANNEL1, packet);
	
	cleanup_l:
	return ERR_OK;
}

int snetwork_callback_maxClients(cfg2_var_t *var, const char *command, lua_State *luaState) {

	if (var->integer < 0) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, 0);
		var->integer = 0;
	}
	if (var->integer > MAX_CLIENTS) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, MAX_CLIENTS);
		var->integer = MAX_CLIENTS;
	}
	g_server.maxClients = var->integer;

	return ERR_OK;
}


static void snetwork_connect(ENetEvent event) {
	ENetPeer *peer = event.peer;
	int index;
	char *ipAddress = NULL;
	
	if (g_server.connectedClients >= g_server.maxClients) {
		// Can't allow any more clients.
		enet_peer_disconnect(event.peer, 0);
		
		network_ipv4ToString(&ipAddress, peer->address.host);
		info("%s:%u attempted to connect, but server is full.", ipAddress, peer->address.port);
		insane_free(ipAddress);
		return;
	}
	
	// Find a suitable client number to use.
	for (index = 0; g_clients[index].inUse; index++);
	
	g_clients[index].peer = peer;
	g_clients[index].inUse = true;
	g_server.connectedClients++;
	
	peer->data = malloc(sizeof(int));
	// Save that index in the peer structure so we can identify it when we recieve messages.
	*((int *) peer->data) = index;
	
	network_ipv4ToString(&ipAddress, peer->address.host);
	info("%s:%u (Client %i) connected", ipAddress, peer->address.port, index);
	insane_free(ipAddress);
}

static int snetwork_receive(ENetEvent event, lua_State *luaState) {
	int error = ERR_CRITICAL;

	// info("Received packet of length %u from %X:%u on channel %u", event.packet->dataLength, event.peer->address.host, event.peer->address.port, event.channelID);
	ENetPeer *peer = event.peer;
	ENetPacket *packet = event.packet;
	int clientNumber;
	ptrdiff_t packetIndex = 0;
	
	// Check for bad guys.
	if (event.peer->data == NULL) {
		// Not a currently connected client.
		error("Unconnected client sent message to server. Ignoring packet.", "");
		enet_packet_destroy(packet);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	clientNumber = *((int *) peer->data);
	
	/* Stack
	*/
	
	// Find the table.
	error = lua_getglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);
	if (error != LUA_TTABLE) {
		// It should have been created by `cnetwork_init`.
		critical_error("Lua file does not contain the table \"%s\".", NETWORK_LUA_CLIENTSTATE_NAME);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	/* Stack
	-1  global table
	*/
	
	error = network_packetRead_lua_object(luaState, packet, &packetIndex);
	if (error) {
		goto cleanup_l;
	}
	
	/* Stack
	-1  table (actual table)
	-2  key (global table name)
	-3  global table
	*/
	
	lua_pushinteger(luaState, clientNumber);
	
	/* Stack
	-1  key (client number, int)
	-2  table (actual table)
	-3  key (global table name, string)
	-4  global table
	*/
	
	lua_copy(luaState, -1, -3);
	
	/* Stack
	-1  key (client number, int)
	-2  table (actual table)
	-3  key (client number, int)
	-4  global table
	*/
	
	lua_pop(luaState, 1);
	
	/* Stack
	-1  table (actual table)
	-2  key (client number, int)
	-3  global table
	*/
	
	lua_settable(luaState, -3);
	
	/* Stack
	-1  global table
	*/
	
	// int top = lua_gettop(luaState);
	// for (int i = 1; i < top; i++)
	// 	printf("%i : %s\n", -i, lua_typename(luaState, lua_type(luaState, -i)));
	
	// lua_common_printTable(luaState);
	
	// lua_pop(luaState, 1);
	// lua_setglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);
	
	/* Stack
	*/
	
	// printf("Client %u: ", clientNumber);
	// for (int i = 0; i < packet->dataLength; i++) {
	// 	if (packet->data[i] < 0x20) {
	// 		printf("<%0X>", packet->data[i]);
	// 	}
	// 	else {
	// 		putc(packet->data[i], stdout);
	// 	}
	// }
	// putc('\n', stdout);
	
	enet_packet_destroy(packet);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

static void snetwork_disconnect(ENetEvent event) {
	
	int index;
	
	if (event.peer->data == NULL) {
		// Not a currently connected client.
		return;
	}
	
	index = *((int *) event.peer->data);
	
	insane_free(event.peer->data);
	if (index < 0) {
		// Not a currently connected client.
		return;
	}
	g_clients[index].inUse = false;
	--g_server.connectedClients;
	info("Client %u disconnected", index);
}

static int snetwork_sendEntityList(void) {
	int error = ERR_CRITICAL;
	
	/*
	{
		g_entityList
		g_entityList->entities
		g_entityList->deletedEntities
		{
			g_entityList->entities->children
		}
	}
	g_entityList->entities->children will be set to the local address of the children array in the packet.
	*/
	
	ENetPacket *packet;
	size_t packetlength = 0;
	enet_uint8 *data;
	ptrdiff_t data_index = 0;
	entityList_t *entityList;
	uint32_t checksum;
	static uint32_t packetID = 0;
	size_t data_length;
		
	packetlength += sizeof(uint32_t);   // Checksum
	packetlength += sizeof(uint32_t);   // Packet counter
	packetlength += sizeof(entityList_t);
	packetlength += g_entityList.entities_length * sizeof(entity_t);
	packetlength += g_entityList.deletedEntities_length * sizeof(int);
	for (int i = 0; i < g_entityList.entities_length; i++) {
		packetlength += g_entityList.entities[i].children_length * sizeof(int);
	}
	
	// Create unreliable packet.
	packet = enet_packet_create(NULL, packetlength, 0);
	if (packet == NULL) {
		critical_error("Out of memory.", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	data = packet->data + sizeof(uint32_t); // packet->data + checksum
	data_length = packetlength - sizeof(uint32_t);
	
	error = network_packetAdd_uint32(data, &data_index, data_length, &packetID, 1);
	if (error) {
		goto cleanup_l;
	}
	packetID++;
	
	entityList = (entityList_t *) &data[data_index];
	error = network_packetAdd_entityList(data, &data_index, data_length, &g_entityList, 1);
	if (error) {
		goto cleanup_l;
	}
	
	entityList->entities = (entity_t *) (&data[data_index] - packet->data);
	error = network_packetAdd_entity(data, &data_index, data_length, g_entityList.entities, g_entityList.entities_length);
	if (error) {
		goto cleanup_l;
	}
	
	entityList->deletedEntities = (ptrdiff_t *) (&data[data_index] - packet->data);
	error = network_packetAdd_ptrdiff(data, &data_index, data_length, g_entityList.deletedEntities, g_entityList.deletedEntities_length);
	if (error) {
		goto cleanup_l;
	}
	
	for (int i = 0; i < g_entityList.entities_length; i++) {
	
		error = network_packetAdd_ptrdiff(data, &data_index, data_length, g_entityList.entities[i].children, g_entityList.entities[i].children_length);
		if (error) {
			goto cleanup_l;
		}	
	}
	
	checksum = network_generateChecksum(data, data_length);
	// Put checksum at the start of the packet.
	// network_packetAdd_uint32 is not used here because this variable is at the beginning, not the end of the packet.
	memcpy(packet->data, &checksum, sizeof(uint32_t));
	
	// Send packet to all clients.
	enet_host_broadcast(g_server.host, ENET_CHANNEL0, packet);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int snetwork_runEvents(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ENetEvent event;
	
	error = snetwork_sendEntityList();
	if (error) {
		goto cleanup_l;
	}
	
	// Run event actions.
	while (enet_host_service(g_server.host, &event, 0) > 0) {
		switch (event.type) {
		case ENET_EVENT_TYPE_CONNECT:
			snetwork_connect(event);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			error = snetwork_receive(event, luaState);
			if (error >= ERR_CRITICAL) {
				goto cleanup_l;
			}
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			snetwork_disconnect(event);
			break;
		default:
			critical_error("Can't happen.", "");
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int snetwork_init(void) {
	int error = ERR_CRITICAL;
	
	error = enet_initialize() != 0;
	if (error != 0) {
		critical_error("Could not initialize ENet.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	g_server.ipAddress.host = ENET_HOST_ANY;
	// Port set by cvar handle.
	
	// Create UDP server.
	// @TODO: Decide if we need to limit bandwidth.
	g_server.host = enet_host_create(&g_server.ipAddress, MAX_CLIENTS, ENET_CHANNELS, 0, 0);
	if (g_server.host == NULL) {
		critical_error("Could not create ENet server.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	enet_host_compress_with_range_coder(g_server.host);
	
	// Initialize client array.
	for (int i = 0; i < MAX_CLIENTS; i++) {
		g_clients[i].inUse = false;
		g_clients[i].peer = NULL;
	}

	error = ERR_OK;
	cleanup_l:
	return error;
}

void snetwork_quit(void) {
	
	// Free client data.
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if ((g_clients[i].peer != NULL) && (g_clients[i].peer->data != NULL)) {
			insane_free(g_clients[i].peer->data);
		}
	}
	
	// End ENet.
	enet_host_destroy(g_server.host);
	enet_deinitialize();
	
}
