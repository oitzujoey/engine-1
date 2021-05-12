
#include "cnetwork.h"
#include <string.h>
#include <stdio.h>
#include "../common/log.h"
#include "../common/insane.h"
#include "../common/network.h"
#include "../common/entity.h"
#include "../common/vector.h"
#include "../common/str2.h"

// UDPsocket g_serverSocket;
// UDPsocket g_clientSocket;
// // SDLNet_SocketSet socketSet_g;

ENetHost *clientHost;
ENetPeer *serverPeer;
cnetwork_clientState_t g_clientState;

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
	
	const ENetPacket *packet;
	int length;
	const enet_uint8 *data;
	ptrdiff_t data_index = 0;
	size_t data_length;
	// entityList and entities in the packet must be preserved for checksum calculation, so we use these copies instead.
	entityList_t entityList;
	entity_t *entities;
	uint32_t checksum, calculatedChecksum;
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
	data_length = length - sizeof(uint32_t);
	
	error = network_packetRead_uint32(&packetID, 1, data, &data_index, data_length);
	if (error) {
		goto cleanup_l;
	}
	
	error = network_packetRead_entityList(&entityList, 1, data, &data_index, data_length);
	if (error) {
		goto cleanup_l;
	}
	
	// Allocate more space for entities if needed.
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
	
	// Allocate more space for deleted entity indices of needed.
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
	
	g_entityList = entityList;
	
	
	entities = calloc(g_entityList.entities_length, sizeof(entity_t));
	if (entities == NULL) {
		critical_error("Out of memory", "");
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	error = network_packetRead_entity(entities, g_entityList.entities_length, data, &data_index, data_length);
	if (error) {
		goto cleanup_l;
	}
	
	// Allocate more space for entities if needed.
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
	
	// Copy entities into main entity list.
	memcpy(g_entityList.entities, entities, g_entityList.entities_length * sizeof(entity_t));
	free(entities);
	
	
	error = network_packetRead_ptrdiff(g_entityList.deletedEntities, g_entityList.deletedEntities_length, data, &data_index, data_length);
	if (error) {
		goto cleanup_l;
	}
	
	for (int i = 0; i < g_entityList.entities_length; i++) {
	
		error = network_packetRead_ptrdiff(g_entityList.entities[i].children, g_entityList.entities[i].children_length, data, &data_index, data_length);
		if (error) {
			goto cleanup_l;
		}
	}
	
	if (lastPacketID + 1 < packetID) {
		warning("Lost %lu packets.", packetID - lastPacketID - 1);
	}
	else if (lastPacketID + 1 < packetID) {
		warning("Possibly %lu out-of-order packets.", lastPacketID + 1 - packetID);
	}
	
	lastPacketID = packetID;
	
	calculatedChecksum = network_generateChecksum(data, data_length);
	
	if (checksum != calculatedChecksum) {
		warning("Calculated bad checksum of %0X. Should be %0X", calculatedChecksum, checksum);
	}
	
	error = ERR_OK;
	cleanup_l:
	
	return error;
}

static int cnetwork_receive(ENetEvent event) {
	int error = ERR_CRITICAL;
	
	char *string = NULL;
	
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
		string = (char *) event.packet->data;
		str2_print(string);
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
