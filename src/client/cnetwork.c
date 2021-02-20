
#include "cnetwork.h"
#include <string.h>
#include <stdio.h>
#include "../common/log.h"
#include "../common/insane.h"
#include "../common/network.h"
#include "../common/str.h"
#include "../common/entity.h"
#include "../common/vector.h"

// UDPsocket g_serverSocket;
// UDPsocket g_clientSocket;
// // SDLNet_SocketSet socketSet_g;

ENetHost *clientHost;
ENetPeer *serverPeer;
cnetwork_clientState_t g_clientState;


// int cnetwork_closeSocket(UDPsocket socket) {
// 	int error;
	
// 	if (socket == NULL) {
// 		error("Socket is NULL", "");
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	// error = SDLNet_UDP_DelSocket(socketSet_g, socket);
// 	// if (error == -1) {
// 	// 	error("SDLNet_UDP_DelSocket returned %s", SDL_GetError());
// 	// 	// Not that we care... The socket will be freed either way.
// 	// }
	
// 	SDLNet_UDP_Close(socket);
// 	socket = NULL;
	
// 	error = 0;
// 	cleanup_l:
	
// 	return error;
// }

// int l_cnetwork_receive(Uint8 **data, int *length) {
// 	int error;
	
// 	UDPpacket *packet = SDLNet_AllocPacket(100);
// 	if (packet == NULL) {
// 		error("SDLNet_AllocPacket returned %s", SDL_GetError());
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	error = SDLNet_UDP_Recv(g_clientSocket, packet);
// 	if (error < 0) {
// 		error("SDLNet_UDP_Recv returned %s", SDL_GetError());
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
// 	if (error > 0) {
// 		// We got something!
// 		*data = realloc(*data, packet->len * sizeof(Uint8));
// 		memcpy(*data, packet->data, packet->len);
// 		*length = packet->len;
// 	}
// 	else {
// 		*length = 0;
// 		// insane_free(data);
// 	}
	
// 	error = 0;
// 	cleanup_l:
	
// 	SDLNet_FreePacket(packet);
	
// 	return error;
// }

// // int l_cnetwork_send(const uint8_t *data, int length, IPaddress ipAddress) {
// // 	int error;
	
// // 	UDPpacket *packet = SDLNet_AllocPacket(length);
// // 	if (packet == NULL) {
// // 		error("SDLNet_AllocPacket returned %s", SDL_GetError());
// // 		error = ERR_GENERIC;
// // 		goto cleanup_l;
// // 	}
	
// // 	memcpy(packet->data, data, length);
// // 	packet->len = length;
	
// // 	packet->address = ipAddress;
	
// // 	error = SDLNet_UDP_Send(g_serverSocket, -1, packet);
	
// // 	if (error == 0) {
// // 		error("SDLNet_UDP_Send returned %s", SDL_GetError());
// // 		error = ERR_GENERIC;
// // 		goto cleanup_l;
// // 	}
	
// // 	error = 0;
// // 	cleanup_l:
	
// // 	SDL_free(packet);
	
// // 	return error;
// // }

// int cnetwork_init(void) {
// 	int error;
	
// 	IPaddress ipAddress;
// 	cfg_var_t *varClientPort;
// 	cfg_var_t *varServerPort;
// 	cfg_var_t *varIpAddress;
	
// 	error = SDLNet_Init();
// 	if (error == -1) {
// 		critical_error("SDLNet_Init returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	varClientPort = cfg_findVar("client_port");
// 	if (varClientPort == NULL) {
// 		critical_error("\"client_port\" undefined", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	g_clientSocket = SDLNet_UDP_Open(varClientPort->integer);
// 	if (g_clientSocket == NULL) {
// 		critical_error("SDLNet_UDP_Open returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	varServerPort = cfg_findVar("server_port");
// 	if (varServerPort == NULL) {
// 		critical_error("\"server_port\" undefined", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	varIpAddress = cfg_findVar("ip_address");
// 	if (varIpAddress == NULL) {
// 		critical_error("\"ip_address undefined", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	error = SDLNet_ResolveHost(&ipAddress, varIpAddress->string.value, varServerPort->integer);
// 	if (error != 0) {
// 		critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	info("Opened UDP socket on port %i", varClientPort->integer);
	
// 	// socketSet_g = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
// 	// if (socketSet_g == NULL) {
// 	// 	critical_error("SDLNet_AllocSocketSet returned %s", SDL_GetError());
// 	// 	error = ERR_CRITICAL;
// 	// 	goto cleanup_l;
// 	// }
	
// 	// error = SDLNet_UDP_AddSocket(socketSet_g, g_serverSocket);
// 	// if (error == -1) {
// 	// 	critical_error("SDLNet_UDP_AddSocket returned %s", SDL_GetError());
// 	// 	error = ERR_CRITICAL;
// 	// 	goto cleanup_l;
// 	// }
	
// 	error = 0;
	
// 	cleanup_l:
	
// 	return error;
// }

// void cnetwork_quit(void) {

// 	// Don't care about errors.
// 	cnetwork_closeSocket(g_clientSocket);
	
// 	// for (int i = 0; i < MAX_CLIENTS; i++) {
// 	// 	if (clients_g[i].socket == NULL) {
// 	// 		continue;
// 	// 	}
// 	// 	// Don't care about errors.
// 	// 	snetwork_closeSocket(clients_g[i].socket);
// 	// }
	
// 	// SDLNet_FreeSocketSet(socketSet_g);
// 	// socketSet_g = NULL;
	
// 	SDLNet_Quit();
// }


/* Config variable handles */

int cnetwork_callback_setIpAddress(cfg2_var_t *var, const char *command, lua_State *luaState) {
	int error = ERR_CRITICAL;

	g_clientState.serverAddressString = var->string;
	error = enet_address_set_host(&g_clientState.serverAddress, var->string);
	if (error < 0) {
		error("Could not set IP address to host \"%s\". Resetting to \"localhost\".", var->string);
		
		var->string = realloc(var->string, (strlen("localhost") + 1) * sizeof(char));
		if (var->string == NULL) {
			critical_error("Out of memory.", "");
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
	enet_peer_reset(serverPeer);
	g_clientState.connected = false;
	g_clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg2.quit = true;
	info("Connection reset", "");
}

static void cnetwork_requestDisconnect(void) {
	enet_peer_disconnect(serverPeer, 0);
	info("Client disconnecting...", "");
}

static void cnetwork_requestConnect(void) {

	info("Requesting to connect with %s:%i", g_clientState.serverAddressString, g_clientState.serverAddress.port);
	serverPeer = enet_host_connect(clientHost, &g_clientState.serverAddress, ENET_CHANNELS, 0);
	g_clientState.lastResponseTime = time(NULL);
	g_clientState.connectionState = cnetwork_connectionState_connecting;
}

static void cnetwork_connect(ENetEvent event) {
	g_clientState.connected = true;
	g_clientState.connectionState = cnetwork_connectionState_connected;
	g_clientState.lastResponseTime = time(NULL);
	info("Successfully connected to %s:%u", g_clientState.serverAddressString, event.peer->address.port);
}

static int cnetwork_receiveEntities(ENetEvent event) {
	int error = ERR_CRITICAL;
	
	ENetPacket *packet;
	int length;
	enet_uint8 *data;
	enet_uint8 *dataStart;
	// entityList and entities in the packet must be preserved for checksum calculation, so we use these copies instead.
	entityList_t entityList;
	entity_t *entities;
	// static quat_t lastQuat = {
	// 	.s = 1,
	// 	.v = {
	// 		0,
	// 		0,
	// 		0
	// 	}
	// };
	uint32_t checksum, calculatedChecksum;
	// static int trap = 0;
	static uint32_t lastPacketID = 0;
	uint32_t packetID;
	
	packet = event.packet;
	data = packet->data;
	length = packet->dataLength;
	
	if (data - packet->data >= length) {
		error("Malformed entity packet", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	memcpy(&checksum, data, sizeof(uint32_t));
	data += sizeof(uint32_t);
	dataStart = data;
	
	memcpy(&packetID, data, sizeof(uint32_t));
	data += sizeof(uint32_t);
	
	memcpy(&entityList, data, sizeof(entityList_t));
	data += sizeof(entityList_t);
	
	if (entityList.entities_length > g_entityList.entities_length) {
		entityList.entities = realloc(g_entityList.entities, entityList.entities_length * sizeof(entity_t));
		if (entityList.entities == NULL) {
			critical_error("Out of memory.", "");
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		
		// Initialize new entities to zero.
		memset((entityList.entities + g_entityList.entities_length), 0, (entityList.entities_length - g_entityList.entities_length) * sizeof(entity_t));
	}
	else {
		entityList.entities = g_entityList.entities;
	}
	
	if (entityList.deletedEntities_length > g_entityList.deletedEntities_length_allocated) {
		entityList.deletedEntities = realloc(g_entityList.deletedEntities, entityList.deletedEntities_length * sizeof(int));
		if (entityList.deletedEntities == NULL) {
			critical_error("Out of memory.", "");
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		entityList.deletedEntities_length_allocated = entityList.deletedEntities_length;
		
	}
	else {
		entityList.deletedEntities = g_entityList.deletedEntities;
	}
	
	memcpy(&g_entityList, &entityList, sizeof(entityList_t));
	
	
	// This uses newly updated length;
	if (data - packet->data >= length) {
		error("Malformed entity packet", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// entities = (entity_t *) data;
	entities = calloc(g_entityList.entities_length, sizeof(entity_t));
	if (entities == NULL) {
		critical_error("Out of memory", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	memcpy(entities, data, g_entityList.entities_length * sizeof(entity_t));
	
	data += g_entityList.entities_length * sizeof(entity_t);
	if (data - packet->data >= length) {
		error("Malformed entity packet", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	for (int i = 0; i < g_entityList.entities_length; i++) {
		if (entities[i].children_length > g_entityList.entities[i].children_length) {
			entities[i].children = realloc(g_entityList.entities[i].children, entities[i].children_length * sizeof(int));
			if (entities[i].children == NULL) {
				critical_error("Out of memory.", "");
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
		}
		else {
			entities[i].children = g_entityList.entities[i].children;
		}
	}
	
	memcpy(g_entityList.entities, entities, g_entityList.entities_length * sizeof(entity_t));
	free(entities);
	
	
	if (data - packet->data >= length) {
		error("Malformed entity packet", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	memcpy(g_entityList.deletedEntities, data, g_entityList.deletedEntities_length * sizeof(int));
	data += g_entityList.deletedEntities_length * sizeof(int);
	
	
	for (int i = 0; i < g_entityList.entities_length; i++) {
		if (data - packet->data >= length) {
			error("Malformed entity packet", "");
			error = ERR_GENERIC;
			goto cleanup_l;
		}
		memcpy(g_entityList.entities[i].children, data, g_entityList.entities[i].children_length * sizeof(int));
		data += g_entityList.entities[i].children_length * sizeof(int);
		// if (i == 1) {
		// 	// quat_print(&g_entityList.entities[i].orientation);
		// 	// printf("%f\n", quat_norm(&g_entityList.entities[i].orientation));
			
		// 	quat_t q0, q1;
		// 	quat_copy(&q0, &lastQuat);
		// 	quat_unitInverse(&q0);
		// 	quat_hamilton(&q1, &g_entityList.entities[i].orientation, &q0);
		// 	quat_copy(&lastQuat, &g_entityList.entities[i].orientation);
		// 	quat_print(&q1);
		// 	if (q1.s < 0.99992) {
		// 		printf("\t");
		// 		quat_print(&g_entityList.entities[i].orientation);
		// 		if (trap == 1) {
		// 			trap++;
		// 		}
		// 		else {
		// 			trap++;
		// 		}
		// 	}
		// }
	}
	
	if (lastPacketID + 1 < packetID) {
		warning("Lost %lu packets.", packetID - lastPacketID - 1);
	}
	else if (lastPacketID + 1 < packetID) {
		warning("Possibly %lu out-of-order packets.", lastPacketID + 1 - packetID);
	}
	
	lastPacketID = packetID;
	
	calculatedChecksum = network_generateChecksum(dataStart, data - dataStart);
	
	if (checksum != calculatedChecksum) {
		warning("Calculated bad checksum of %0X. Should be %0X", calculatedChecksum, checksum);
	}
	fflush(stdout);
	fflush(stderr);
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

static int cnetwork_receive(ENetEvent event) {
	int error = ERR_CRITICAL;
	
	string_t string;
	
	if (g_clientState.connectionState != cnetwork_connectionState_connected) {
		error = ERR_OK;
		goto cleanup_l;
	}
	
	switch (event.channelID) {
	case ENET_CHANNEL0:
		error = cnetwork_receiveEntities(event);
		if (error) {
			goto cleanup_l;
		}
		break;
	case ENET_CHANNEL1:
		string.value = (char *) event.packet->data;
		string.length = event.packet->dataLength;
		string.memsize = 0;
		string_print(&string);
		break;
	default:
		error("Bad ENet channel %i.", event.channelID);
	}
	
	error = ERR_OK;
	cleanup_l:
	
	enet_packet_destroy(event.packet);
	
	return error;
}

static void cnetwork_disconnect(ENetEvent event) {
	g_clientState.connected = false;
	g_clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg2.quit = true;
	info("Client disconnected.", "");
}

int cnetwork_runEvents(void) {
	int error = ERR_CRITICAL;
	
	ENetEvent event;
	
	// Attempt to connect on startup.
	if (g_clientState.connectionState == cnetwork_connectionState_initial) {
		// Note that time is not of the essence when connecting.
		cnetwork_requestConnect();
		error = enet_host_service(clientHost, &event, g_connectionTimeout);
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
		error = enet_host_service(clientHost, &event, g_connectionTimeout);
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
	
	do {
		error = enet_host_service(clientHost, &event, 0);
		
		if (error > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				cnetwork_connect(event);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				cnetwork_receive(event);
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
		if (error < 0) {
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	} while (error != 0);
	
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
	clientHost = enet_host_create(NULL, 1, ENET_CHANNELS, 0, 0);
	if (clientHost == NULL) {
		critical_error("Could not create ENet client.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	enet_host_compress_with_range_coder(clientHost);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

void cnetwork_quit(void) {
	
	enet_host_destroy(clientHost);
	enet_deinitialize();
	
}
