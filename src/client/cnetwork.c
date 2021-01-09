
#include "cnetwork.h"
#include "../common/log.h"
#include "../common/insane.h"
#include "../common/network.h"
#include "../common/str.h"

// UDPsocket g_serverSocket;
// UDPsocket g_clientSocket;
// // SDLNet_SocketSet socketSet_g;

ENetHost *clientHost;
ENetPeer *serverPeer;
cnetwork_clientState_t clientState;


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

int cnetwork_handle_setIpAddress(cfg_var_t *var) {
	int error = ERR_CRITICAL;

	clientState.serverAddressString = var->string.value;
	error = enet_address_set_host(&clientState.serverAddress, var->string.value);
	if (error < 0) {
		error("Could not set IP address to host \"%s\". Resetting to \"localhost\".", var->string.value);
		error = string_copy_c(&var->string, "localhost");
		if (error) {
			critical_error("Out of memory.", "");
			error = ERR_CRITICAL;
			goto cleanup_l;
		}
		error = enet_address_set_host(&clientState.serverAddress, var->string.value);
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

int cnetwork_handle_setServerPort(cfg_var_t *var) {

	// Shouldn't have to check type because it will always be hard coded when this function is run.
	if (var->integer < 1024) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, CFG_CONNECTION_TIMEOUT_DEFAULT);
		var->integer = CFG_CONNECTION_TIMEOUT_DEFAULT;
	}
	clientState.serverAddress.port = var->integer;

	return ERR_OK;
}

int cnetwork_handle_setClientPort(cfg_var_t *var) {

	// Shouldn't have to check type because it will always be hard coded when this function is run.
	if (var->integer < 1024) {
		warning("Variable \"%s\" out of range. Setting to %i.", var->name, CFG_CONNECTION_TIMEOUT_DEFAULT);
		var->integer = CFG_CONNECTION_TIMEOUT_DEFAULT;
	}

	return ERR_OK;
}

/* Networking functions */

static void cnetwork_connectionReset(void) {
	enet_peer_reset(serverPeer);
	clientState.connected = false;
	clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg.quit = true;
	info("Connection reset", "");
}

static void cnetwork_requestDisconnect(void) {
	enet_peer_disconnect(serverPeer, 0);
	info("Client disconnecting...", "");
}

static void cnetwork_requestConnect(void) {

	serverPeer = enet_host_connect(clientHost, &clientState.serverAddress, ENET_CHANNELS, 0);
	clientState.lastResponseTime = time(NULL);
	clientState.connectionState = cnetwork_connectionState_connecting;
}

static void cnetwork_connect(ENetEvent event) {
	clientState.connected = true;
	clientState.connectionState = cnetwork_connectionState_connected;
	clientState.lastResponseTime = time(NULL);
	info("Successfully connected to %s:%u", clientState.serverAddressString, event.peer->address.port);
}

static void cnetwork_receive(ENetEvent event) {
	// info("Received packet of length %u from %s:%u on channel %u", event.packet->dataLength, clientState.serverAddressString, event.peer->address.port, event.channelID);
	string_t string;
	string.value = (char *) event.packet->data;
	string.length = event.packet->dataLength;
	string.memsize = 0;
	string_print(&string);
}

static void cnetwork_disconnect(ENetEvent event) {
	clientState.connected = false;
	clientState.connectionState = cnetwork_connectionState_disconnected;
	g_cfg.quit = true;
	info("Client disconnected.", "");
}

int cnetwork_runEvents(void) {
	int error = ERR_CRITICAL;
	
	ENetEvent event;
	
	// Attempt to connect on startup.
	if (clientState.connectionState == cnetwork_connectionState_initial) {
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
			error("Could not disconnect from server. Resetting connection.", "");
			cnetwork_connectionReset();
			error = ERR_GENERIC;
			goto cleanup_l;
		}
	}
	
	// Reset connection if the timeout is exceeded at any time.
	// if (
	// 	(
	// 		(clientState.connectionState == cnetwork_connectionState_connecting) ||
	// 		(clientState.connectionState == cnetwork_connectionState_connected)
	// 	) &&
	// 	(difftime(time(NULL), clientState.lastResponseTime) >= (double) g_connectionTimeout)
	// ) {
	// 	if (clientState.connectionState == cnetwork_connectionState_connecting) {
	// 		error("Could not connect to %s:%i", clientState.serverAddressString, clientState.serverAddress.port);
	// 	}
	// 	cnetwork_connectionReset();
	// 	error = ERR_OK;
	// 	goto cleanup_l;
	// }
	
	if (g_cfg.quit) {
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
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int cnetwork_init(void) {
	int error = ERR_CRITICAL;
	
	clientState.connected = false;
	clientState.connectionState = cnetwork_connectionState_initial;
	clientState.lastResponseTime = time(NULL);
	
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
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

void cnetwork_quit(void) {
	
	enet_host_destroy(clientHost);
	enet_deinitialize();
	
}
