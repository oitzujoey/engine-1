
#include "cnetwork.h"
#include <string.h>
#include <stdio.h>
#include "../common/log.h"
#include "../common/network.h"
#include "../common/entity.h"
#include "../common/vector.h"
#include "../common/str2.h"
#include "../common/lua_common.h"
#include "../common/memory.h"

// UDPsocket g_serverSocket;
// UDPsocket g_clientSocket;
// // SDLNet_SocketSet socketSet_g;

ENetHost *g_clientHost;
ENetPeer *g_serverPeer;
cnetwork_clientState_t g_clientState;

/* /\* */
/* A packet to send to the server. Will eventually contain keys and things like */
/* that. */
/* *\/ */
/* enet_uint8 *g_clientStateArray = NULL; */
/* int g_clientStateArray_length = 0; */

extern cfg2_t g_cfg2;


/* Config variable handles */

int cnetwork_callback_setIpAddress(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;

	g_clientState.serverAddressString = var->string;
	error = enet_address_set_host(&g_clientState.serverAddress, var->string);
	if (error < 0) {
		error("Could not set IP address to host \"%s\". Resetting to \"localhost\".", var->string);
		
		var->string = realloc(var->string, (strlen("localhost") + 1) * sizeof(char));
		if (var->string == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		strcpy(var->string, "localhost");
		
		error = enet_address_set_host(&g_clientState.serverAddress, var->string);
		if (error) {
			critical_error("Could not set IP address to host \"localhost\".", "");
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int cnetwork_callback_setServerPort(cfg2_var_t *var, const char *command, lua_State *luaState) {

	// Shouldn't have to check type because it will always be hard coded when this function is run.
	if (var->integer < 1024) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, CFG_PORT_DEFAULT);
		var->integer = CFG_PORT_DEFAULT;
	}
	g_clientState.serverAddress.port = var->integer;

	return ERR_OK;
}

/* Networking functions */

static void cnetwork_connectionReset(void) {
	enet_peer_reset(g_serverPeer);
	g_clientState.connected = false;
	g_clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg2.quit = true;
	info("Connection reset", "");
}

static void cnetwork_requestDisconnect(void) {
	enet_peer_disconnect(g_serverPeer, 0);
	info("Client disconnecting...", "");
}

static void cnetwork_requestConnect(void) {

	info("Requesting to connect with %s:%i", g_clientState.serverAddressString, g_clientState.serverAddress.port);
	g_serverPeer = enet_host_connect(g_clientHost, &g_clientState.serverAddress, ENET_CHANNELS, 0);
	g_clientState.lastResponseTime = time(NULL);
	g_clientState.connectionState = cnetwork_connectionState_connecting;
}

static void cnetwork_connect(ENetEvent event) {
	g_clientState.connected = true;
	g_clientState.connectionState = cnetwork_connectionState_connected;
	g_clientState.lastResponseTime = time(NULL);
	info("Successfully connected to %s:%u", g_clientState.serverAddressString, event.peer->address.port);
}

/*
Here we are going to find the "clientState" table and convert it to a byte
array. This is very similar to how the entities are sent to the client.
*/
static int cnetwork_sendState(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ENetPacket *packet = NULL;
	ptrdiff_t index = 0;
	
	// Find the table.
	error = lua_getglobal(luaState, NETWORK_LUA_CLIENTSTATE_NAME);
	if (error != LUA_TTABLE) {
		// It should have been created by `cnetwork_init`.
		critical_error("Lua file does not contain the table \"%s\".", NETWORK_LUA_CLIENTSTATE_NAME);
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	// // It might be empty. That's fine. Don't send.
	// if (g_clientStateArray == NULL || g_clientStateArray_length == 0) {
	// 	if (g_clientStateArray != NULL && g_clientStateArray_length == 0) {
	// 		// Or we could have a bad pointer.
	// 		critical_error("Array has length of 0 but is not NULL.", "");
	// 		error = ERR_CRITICAL;
	// 	} else {
	// 		error = ERR_OK;
	// 	}
	// 	goto cleanup_l;
	// }
	
	// Create the packet.
	packet = enet_packet_create(NULL, 0, 0);
	if (packet == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Send the table.
	error = network_packetAdd_lua_object(luaState, NETWORK_LUA_CLIENTSTATE_NAME, network_lua_type_string, packet, &index);
	if (error) {
		goto cleanup_l;
	}
	
	lua_pop(luaState, 1);
	
	// Send to the server.
	error = enet_peer_send(g_serverPeer, ENET_CHANNEL0, packet);
	if (error < 0) {
		error("Unable to send packet to server.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:

	/* MEMORY_FREE(&g_clientStateArray); */
	/* g_clientStateArray_length = 0; */
	
	return error;
}

static int cnetwork_receive(ENetEvent event, lua_State *luaState) {
	int e = ERR_OK;

	ENetPacket *packet;
	// int length;
	// const enet_uint8 *data;
	ptrdiff_t data_index = 0;
	uint32_t checksum, calculatedChecksum;
	static uint32_t lastPacketID = 0;
	uint32_t packetID;

	if (g_clientState.connectionState != cnetwork_connectionState_connected) {
		goto cleanup;
	}

	packet = event.packet;

	(void) memcpy(&checksum, packet->data, sizeof(uint32_t));
	data_index += sizeof(uint32_t);

	error = network_packetRead_uint32(&packetID, 1, packet->data, &data_index, packet->dataLength);
	if (error) goto cleanup;

	// Server state

	// stack: serverState::Table

	error = network_packetRead_lua_object(luaState, packet, &data_index);
	if (error) goto cleanup;
	// stack: serverState

	// Add to the array of messages.
	// stack: serverState global-name? object
	(void) lua_pushinteger(luaState, lua_rawlen(luaState, -3) + 1);
	// stack: serverState global-name? object #serverState+1
	(void) lua_pushvalue(luaState, -2);
	// stack: serverState global-name? object #serverState+1 object
	(void) lua_settable(luaState, -5);
	// stack: serverState global-name? object
	(void) lua_pop(luaState, 2);
	// stack: serverState

	if (lastPacketID + 1 < packetID) {
		warning("Lost %lu packets.", packetID - lastPacketID - 1);
	}
	else if (lastPacketID + 1 < packetID) {
		warning("Possibly %lu out-of-order packets.", lastPacketID + 1 - packetID);
	}

	lastPacketID = packetID;

	// Calculate the checksum while ignoring the recorded checksum.
	calculatedChecksum = network_generateChecksum(packet->data + sizeof(uint32_t), packet->dataLength - sizeof(uint32_t));

	if (checksum != calculatedChecksum) {
		warning("Calculated bad checksum of %0X. Should be %0X", calculatedChecksum, checksum);
	}

 cleanup:
	(void) enet_packet_destroy(event.packet);
	return e;
}

static void cnetwork_disconnect(ENetEvent event) {
	g_clientState.connected = false;
	g_clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg2.quit = true;
	info("Client disconnected.", "");
}

int cnetwork_runEvents(lua_State *luaState) {
	int error = ERR_CRITICAL;
	int enetStatus = 0;
	
	ENetEvent event;
	
	// Attempt to connect on startup.
	if (g_clientState.connectionState == cnetwork_connectionState_initial) {
		// Note that time is not of the essence when connecting.
		cnetwork_requestConnect();
		error = enet_host_service(g_clientHost, &event, g_connectionTimeout);
		// Connected.
		if ((error > 0) && (event.type == ENET_EVENT_TYPE_CONNECT)) {
			cnetwork_connect(event);
			error = ERR_OK;
			goto cleanup_l;
		}
		// Failed to connect. Reset the connection.
		else {
			error("Could not connect to server. Resetting connection.", "");
			cnetwork_connectionReset();
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	// // Reset connection if the timeout is exceeded at any time.
	// if (
	// 	(
	// 		(g_clientState.connectionState == cnetwork_connectionState_connecting) ||
	// 		(g_clientState.connectionState == cnetwork_connectionState_connected)
	// 	) &&
	// 	(difftime(time(NULL), g_clientState.lastResponseTime) >= (double) g_connectionTimeout / 1000.0)
	// ) {
	// 	if (g_clientState.connectionState == cnetwork_connectionState_connecting) {
	// 		error("Could not connect to %s:%i", g_clientState.serverAddressString, g_clientState.serverAddress.port);
	// 	}
	// 	cnetwork_connectionReset();
	// 	error = ERR_OK;
	// 	goto cleanup_l;
	// }
	
	if (g_cfg2.quit) {
		cnetwork_requestDisconnect();
		error = enet_host_service(g_clientHost, &event, g_connectionTimeout);
		// Disconnected successfully.
		if ((error > 0) && (event.type == ENET_EVENT_TYPE_DISCONNECT)) {
			cnetwork_disconnect(event);
			error = ERR_OK;
			goto cleanup_l;
		}
		// Failed to disconnect. Force it.
		else {
			error("Could not disconnect from server. Resetting connection.", "");
			cnetwork_connectionReset();
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	if (g_clientState.connectionState == cnetwork_connectionState_connected) {
		error = cnetwork_sendState(luaState);
		if (error > ERR_GENERIC) {
			goto cleanup_l;
		}
	}

	// Empty serverState.
	(void) lua_createtable(luaState, MAX_CLIENTS, 0);
	// stack: serverState::Table

	do {
		enetStatus = enet_host_service(g_clientHost, &event, 0);
		
		if (enetStatus > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				cnetwork_connect(event);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				// stack: serverState::Table
				error = cnetwork_receive(event, luaState);
				if (error > ERR_GENERIC) goto cleanup_l;
				// stack: serverState
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				cnetwork_disconnect(event);
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

	// Set serverState.
	(void) lua_setglobal(luaState, NETWORK_LUA_SERVERSTATE_NAME);

	error = ERR_OK;
	cleanup_l:
	return error;
}

int cnetwork_init(void) {
	int error = ERR_CRITICAL;
	
	// cfg2_var_t *v_ipAddress;
	
	g_clientState.connected = false;
	g_clientState.connectionState = cnetwork_connectionState_initial;
	g_clientState.lastResponseTime = time(NULL);
	
	// v_ipAddress = cfg2_findVar(CFG_IP_ADDRESS);
	// if (v_ipAddress == NULL) {
	// 	critical_error("Variable \""CFG_IP_ADDRESS"\" does not exist.", "");
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	// error = cnetwork_callback_setIpAddress(v_ipAddress, "", NULL);
	// if (error) {
	// 	goto cleanup_l;
	// }
	
	error = enet_initialize() != 0;
	if (error != 0) {
		critical_error("Could not initialize ENet.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// Create UDP client.
	// @TODO: Decide if we need to limit bandwidth.
	g_clientHost = enet_host_create(NULL, 1, ENET_CHANNELS, 0, 0);
	if (g_clientHost == NULL) {
		critical_error("Could not create ENet client.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = enet_host_compress_with_range_coder(g_clientHost);
	if (error < 0) {
		critical_error("Could not enable the packet compressor.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

void cnetwork_quit(void) {
	
	enet_host_destroy(g_clientHost);
	enet_deinitialize();
	
}
