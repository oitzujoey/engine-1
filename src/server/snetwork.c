
#include "snetwork.h"
#include <stdio.h>
#include <string.h>
#include "../common/log.h"
#include "../common/network.h"
#include "server.h"
#include "../common/entity.h"
#include "../common/vector.h"
#include "../common/lua_common.h"
#include "../common/lua_sandbox.h"
#include "../common/memory.h"

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


static int snetwork_connect(ENetEvent event, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ENetPeer *peer = event.peer;
	int index;
	char *ipAddress = NULL;
	
	if (g_server.connectedClients >= g_server.maxClients) {
		// Can't allow any more clients.
		enet_peer_disconnect(event.peer, 0);
		
		network_ipv4ToString(&ipAddress, peer->address.host);
		info("%s:%u attempted to connect, but server is full.", ipAddress, peer->address.port);
		MEMORY_FREE(&ipAddress);
		error = ERR_OK;
		goto cleanup_l;
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
	MEMORY_FREE(&ipAddress);
	
	
	luaTimeout_t luaTimeout = {
		.functionName = "clientConnect",
		.luaState = luaState
	};
	
	error = lua_getglobal(luaState, luaTimeout.functionName);
	if (error != LUA_TFUNCTION) {
		error("Lua file does not contain \"%s\" function.", luaTimeout.functionName);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	lua_pushinteger(luaState, index + 1);
	
	SDL_TimerID timerId = SDL_AddTimer(100, lua_luaTimeout, &luaTimeout);
	
	error = lua_pcall(luaState, 1, 0, 0);
	SDL_RemoveTimer(timerId);
	if (error) {
		lua_sandbox_handleError(luaState);
		error("Lua function \"%s\" exited with error %s", luaTimeout.functionName, luaError[error]);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

static int snetwork_receive(ENetEvent event, lua_State *luaState) {
	int e = ERR_OK;

	// info("Received packet of length %u from %X:%u on channel %u", event.packet->dataLength, event.peer->address.host, event.peer->address.port, event.channelID);
	ENetPeer *peer = event.peer;
	ENetPacket *packet = event.packet;
	int clientNumber;
	ptrdiff_t packetIndex = 0;
	
	// static uint32_t lastPacketID[8] = {0};
	// uint32_t packetID;
	
	// Check for bad guys.
	if (event.peer->data == NULL) {
		// Not a currently connected client.
		error("Unconnected client sent message to server. Ignoring packet.", "");
		enet_packet_destroy(packet);
		e = ERR_GENERIC;
		goto cleanup;
	}
	
	clientNumber = *((int *) peer->data);

	// Fetch message buffer table for client.
	// stack: clientStates[MAX_CLIENTS]::Table[Table]
	(void) lua_pushinteger(luaState, clientNumber + 1);
	// stack: clientStates clientNumber
	(void) lua_gettable(luaState, -2);
	// stack: clientStates clientState

	// Pass: Nothing.
	// Return: global-name? object
	e = network_packetRead_lua_object(luaState, packet, &packetIndex);
	if (e) goto cleanup;
	// stack: clientStates clientState key object
	(void) lua_pushinteger(luaState, lua_rawlen(luaState, -3) + 1);
	// stack: clientStates clientState key object #clientState+1
	(void) lua_pushvalue(luaState, -2);
	// stack: clientStates clientState key object #clientState+1 object
	(void) lua_settable(luaState, -5);
	// stack: clientStates clientState key object
	(void) lua_pop(luaState, 2);
	// stack: clientStates clientState
	(void) lua_pushinteger(luaState, clientNumber + 1);
	// stack: clientStates clientState clientNumber
	(void) lua_pushvalue(luaState, -2);
	// stack: clientStates clientState clientNumber clientState
	(void) lua_settable(luaState, -4);
	// stack: clientStates clientState
	(void) lua_pop(luaState, 1);
	// stack: clientStates[MAX_CLIENTS]::Table[Table]

	(void) enet_packet_destroy(packet);

 cleanup: return e;
}

static int snetwork_disconnect(ENetEvent event, lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int index;
	
	if (event.peer->data == NULL) {
		// Not a currently connected client.
		error = ERR_OK;
		goto cleanup_l;
	}
	
	index = *((int *) event.peer->data);
	
	MEMORY_FREE(&event.peer->data);
	if (index < 0) {
		// Not a currently connected client.
		error = ERR_OK;
		goto cleanup_l;
	}
	g_clients[index].inUse = false;
	--g_server.connectedClients;
	info("Client %u disconnected", index);
	
	
	luaTimeout_t luaTimeout = {
		.functionName = "clientDisconnect",
		.luaState = luaState
	};
	
	error = lua_getglobal(luaState, luaTimeout.functionName);
	if (error != LUA_TFUNCTION) {
		error("Lua file does not contain \"%s\" function.", luaTimeout.functionName);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	lua_pushinteger(luaState, index + 1);
	
	SDL_TimerID timerId = SDL_AddTimer(100, lua_luaTimeout, &luaTimeout);
	
	error = lua_pcall(luaState, 1, 0, 0);
	SDL_RemoveTimer(timerId);
	if (error) {
		lua_sandbox_handleError(luaState);
		error("Lua function \"%s\" exited with error %s", luaTimeout.functionName, luaError[error]);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

static int snetwork_sendEntityList(lua_State *luaState) {
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
	
	static uint32_t packetID[MAX_CLIENTS] = {0};
	
	ENetPacket *packet;
	size_t packetlength;
	// enet_uint8 *data;
	ptrdiff_t data_index;
	entityList_t *entityList;
	uint32_t checksum;
	// size_t data_length;
	
	
	// Find the table.
	error = lua_getglobal(luaState, NETWORK_LUA_SERVERSTATE_NAME);
	if (error != LUA_TTABLE) {
		// It should have been created by `cnetwork_init`.
		critical_error("Lua file does not contain the table \"%s\".", NETWORK_LUA_SERVERSTATE_NAME);
		error = ERR_CRITICAL;
		goto cleanupNoPop_l;
	}
	/*
	-1  serverState[]
	*/
	
	for (int clientNumber = 0; clientNumber < MAX_CLIENTS; clientNumber++) {
		if (!g_clients[clientNumber].inUse) {
			continue;
		}
		
		packetlength = 0;
		data_index = 0;
	
		packetlength += sizeof(uint32_t);   // Checksum
		packetlength += sizeof(uint32_t);   // Packet counter
		
		// Create unreliable packet.
		packet = enet_packet_create(NULL, packetlength, 0);
		if (packet == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}

		// Checksum
		data_index += sizeof(uint32_t);

		// Packet number
		error = network_packetAdd_uint32(packet->data, &data_index, packet->dataLength, &packetID[clientNumber], 1);
		if (error) {
			goto enet_cleanup_l;
		}
		packetID[clientNumber]++;
		
		// Find this client's serverState.
		lua_pushinteger(luaState, clientNumber + 1);
		/*
		-1  clientNumber (one-based)
		-2  serverState[]
		*/
		lua_gettable(luaState, -2);
		/*
		-1  serverState[clientNumber]
		-2  serverState[]
		*/
		
		// Add the table to the packet. I think this should take care of the packet malloc just fine.
		error = network_packetAdd_lua_object(luaState, NETWORK_LUA_SERVERSTATE_NAME, network_lua_type_string, packet, &data_index);
		if (error) {
			goto cleanup_l;
		}
		
		lua_pop(luaState, 1);
		/*
		-1  serverState[]
		*/
		
		
		checksum = network_generateChecksum(packet->data + sizeof(uint32_t), packet->dataLength - sizeof(uint32_t));
		// Put checksum at the start of the packet.
		// network_packetAdd_uint32 is not used here because this variable is at the beginning, not the end of the packet.
		memcpy(packet->data, &checksum, sizeof(uint32_t));
		
		// Send packet to all clients.
		// enet_host_broadcast(g_server.host, ENET_CHANNEL0, packet);
		error = enet_peer_send(g_clients[clientNumber].peer, ENET_CHANNEL0, packet);
		if (error < 0) {
			error("Unable to send packet to client %i.", clientNumber);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	error = ERR_OK;
	goto cleanup_l;
	
	enet_cleanup_l:
	lua_pop(luaState, 1);
	enet_free(packet);
	
	cleanup_l:
	lua_pop(luaState, 1);
	
	cleanupNoPop_l:
	return error;
}

int snetwork_runEvents(lua_State *luaState) {
	int error = ERR_CRITICAL;
	int enetStatus = 0;
	
	ENetEvent event;
	
	error = snetwork_sendEntityList(luaState);
	if (error) {
		goto cleanup_l;
	}

	// Empty clientState.
	(void) lua_createtable(luaState, MAX_CLIENTS, 0);
	// stack: clientState::Table
	// Create a state buffer for each client.
	for (lua_Integer i = 1; i <= MAX_CLIENTS; i++) {
		(void) lua_pushinteger(luaState, i);
		// stack: clientState i
		(void) lua_newtable(luaState);
		// stack: clientState i {}
		(void) lua_settable(luaState, -3);
		// stack: clientState[i]::Table[Table]
	}
	// stack: clientState[MAX_CLIENTS]::Table[Table]

	// Run event actions.
	do {
		enetStatus = enet_host_service(g_server.host, &event, 0);
		
		if (enetStatus > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				error = snetwork_connect(event, luaState);
				if (error) {
					goto cleanup_l;
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				// stack: clientState[MAX_CLIENTS]::Table[Table]
				error = snetwork_receive(event, luaState);
				if (error >= ERR_CRITICAL) {
					goto cleanup_l;
				}
				// stack: clientState
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				error = snetwork_disconnect(event, luaState);
				if (error) {
					goto cleanup_l;
				}
				break;
			default:
				critical_error("Can't happen.", "");
				error = ERR_CRITICAL;
				goto cleanup_l;
			}
		}
		if (enetStatus < 0) {
			error("enet_host_service encountered an error: %i", error);
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	} while (enetStatus != 0);

	// Set clientState.
	(void) lua_setglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);

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
			MEMORY_FREE(&g_clients[i].peer->data);
		}
	}
	
	// End ENet.
	enet_host_destroy(g_server.host);
	enet_deinitialize();
	
}
