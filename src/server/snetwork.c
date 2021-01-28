
#include "snetwork.h"
#include <stdio.h>
#include <string.h>
#include "../common/log.h"
#include "../common/insane.h"
#include "../common/network.h"
#include "server.h"
#include "../common/entity.h"
#include "../common/vector.h"

// client_t g_clients[MAX_CLIENTS];
// UDPsocket g_serverSocket;
// // SDLNet_SocketSet g_socketSet;

serverState_t g_server;
client_t g_clients[MAX_CLIENTS];


int snetwork_handle_setServerPort(cfg_var_t *var) {

	// Shouldn't have to check type because it will always be hard coded when this function is run.
	if (var->integer < 1024) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, CFG_PORT_DEFAULT);
		var->integer = CFG_PORT_DEFAULT;
	}
	g_server.ipAddress.port = var->integer;

	return ERR_OK;
}

int snetwork_handle_enetMessage(cfg_var_t *var) {

	if (var->string.length == 0) {
		goto cleanup_l; 
	}
	
	ENetPacket *packet = enet_packet_create(var->string.value, var->string.length + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(g_server.host, ENET_CHANNEL1, packet);
	
	cleanup_l:
	return ERR_OK;
}

int snetwork_handle_maxClients(cfg_var_t *var) {

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


// int snetwork_closeSocket(UDPsocket socket) {
// 	int error;
	
// 	if (socket == NULL) {
// 		error("Socket is NULL", "");
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	// error = SDLNet_UDP_DelSocket(g_socketSet, socket);
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

// int l_snetwork_send(const uint8_t *data, int length, IPaddress ipAddress) {
// 	int error;
	
// 	UDPpacket *packet = SDLNet_AllocPacket(length);
// 	if (packet == NULL) {
// 		error("SDLNet_AllocPacket returned %s", SDL_GetError());
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	memcpy(packet->data, data, length);
// 	packet->len = length;
	
// 	packet->address = ipAddress;
	
// 	error = SDLNet_UDP_Send(g_serverSocket, -1, packet);
	
// 	if (error == 0) {
// 		error("SDLNet_UDP_Send returned %s", SDL_GetError());
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	error = 0;
// 	cleanup_l:
	
// 	SDLNet_FreePacket(packet);
	
// 	return error;
// }

// int snetwork_init(void) {
// 	int error;
	
// 	IPaddress ipAddress;
// 	cfg_var_t *varServerPort;
// 	cfg_var_t *varClientPort;
	
// 	error = SDLNet_Init();
// 	if (error == -1) {
// 		critical_error("SDLNet_Init returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	varServerPort = cfg_findVar("server_port");
// 	if (varServerPort == NULL) {
// 		critical_error("\"server_port\" undefined", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	g_serverSocket = SDLNet_UDP_Open(varServerPort->integer);
// 	if (g_serverSocket == NULL) {
// 		critical_error("SDLNet_UDP_Open returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	varClientPort = cfg_findVar("client_port");
// 	if (varClientPort == NULL) {
// 		critical_error("\"client_port\" undefined", "");
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	error = SDLNet_ResolveHost(&ipAddress, NULL, varClientPort->integer);
// 	if (error != 0) {
// 		critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
// 		error = ERR_CRITICAL;
// 		goto cleanup_l;
// 	}
	
// 	info("Opened UDP socket on port %i", varServerPort->integer);
	
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

// void snetwork_quit(void) {

// 	// Don't care about errors.
// 	snetwork_closeSocket(g_serverSocket);
	
// 	for (int i = 0; i < MAX_CLIENTS; i++) {
// 		if (g_clients[i].socket == NULL) {
// 			continue;
// 		}
// 		// Don't care about errors.
// 		snetwork_closeSocket(g_clients[i].socket);
// 	}
	
// 	// SDLNet_FreeSocketSet(socketSet_g);
// 	// socketSet_g = NULL;
	
// 	SDLNet_Quit();
// }

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

static void snetwork_receive(ENetEvent event) {
	// info("Received packet of length %u from %X:%u on channel %u", event.packet->dataLength, event.peer->address.host, event.peer->address.port, event.channelID);
	ENetPeer *peer = event.peer;
	ENetPacket *packet = event.packet;
	int index;
	
	if (event.peer->data == NULL) {
		// Not a currently connected client.
		enet_packet_destroy(packet);
		return;
	}
	
	index = *((int *) peer->data);
	
	printf("Client %u: ", index);
	for (int i = 0; i < packet->dataLength; i++) {
		putc(packet->data[i], stdout);
	}
	putc('\n', stdout);
	enet_packet_destroy(packet);
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
	enet_uint8 *dataStart;
	entityList_t *entityList;
	entity_t *entities;
	uint32_t checksum;
	static quat_t lastQuat = {
		.s = 1,
		.v = {
			0,
			0,
			0
		}
	};
	static uint32_t packetID = 0;
		
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
	dataStart = data;
	
	memcpy(data, &packetID, sizeof(uint32_t));
	packetID++;
	data += sizeof(uint32_t);
	
	memcpy(data, &g_entityList, sizeof(entityList_t));
	entityList = (entityList_t *) data;
	data += sizeof(entityList_t);
	
	// for (int i = 0; i < g_entityList.entities_length; i++) {
	// 	printf("%i children         %lu\n", i, (unsigned long) g_entityList.entities[i].children);
	// 	printf("%i children_length  %i\n", i, g_entityList.entities[i].children_length);
	// 	printf("%i childType        %i\n", i, g_entityList.entities[i].childType);
	// 	for (int j = 0; j < 3; j++) {
	// 	printf("%i orientation.v[%i] %f\n", i, j, g_entityList.entities[i].orientation.v[j]);
	// 	}
	// 	printf("%i orientation.s    %f\n", i, g_entityList.entities[i].orientation.s);
	// 	for (int j = 0; j < 3; j++) {
	// 	printf("%i position[%i]      %f\n", i, j, g_entityList.entities[i].position[j]);
	// 	}
	// 	puts("");
	// }
	// 76
	memcpy(data, g_entityList.entities, g_entityList.entities_length * sizeof(entity_t));
	entityList->entities = (entity_t *) (data - packet->data);
	entities = (entity_t *) data;
	data += g_entityList.entities_length * sizeof(entity_t);
	
	// printf("data %lu\n", (unsigned long) (data - entities));
	
	memcpy(data, g_entityList.deletedEntities, g_entityList.deletedEntities_length * sizeof(int));
	entityList->deletedEntities = (int *) (data - packet->data);
	data += g_entityList.deletedEntities_length * sizeof(int);
	
	for (int i = 0; i < g_entityList.entities_length; i++) {
		// Copy children indices.
		memcpy(data, g_entityList.entities[i].children, g_entityList.entities[i].children_length * sizeof(int));
		entities[i].children = (int *) (data - packet->data);
		data += g_entityList.entities[i].children_length * sizeof(int);
		if (i == 1) {
			// quat_print(&g_entityList.entities[i].orientation);
			// printf("%f\n", quat_norm(&g_entityList.entities[i].orientation));
			
			quat_t q0, q1;
			quat_copy(&q0, &lastQuat);
			quat_unitInverse(&q0);
			quat_hamilton(&q1, &g_entityList.entities[i].orientation, &q0);
			quat_copy(&lastQuat, &g_entityList.entities[i].orientation);
			// quat_print(&q1);
			// if (q1.s < 0.99992) {
			// 	printf("\t");
			// 	quat_print(&g_entityList.entities[i].orientation);
			// }
		}
	}
	
	// Put checksum at the start of the packet.
	checksum = network_generateChecksum(dataStart, data - dataStart);
	memcpy(packet->data, &checksum, sizeof(uint32_t));
	
	enet_host_broadcast(g_server.host, ENET_CHANNEL0, packet);
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int snetwork_runEvents(void) {
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
			snetwork_receive(event);
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
