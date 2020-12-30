
#include "snetwork.h"
#include "../common/log.h"
#include "../common/cfg.h"
#include "../common/insane.h"

client_t clients_g[MAX_CLIENTS];
UDPsocket g_serverSocket;
// SDLNet_SocketSet socketSet_g;


int snetwork_closeSocket(UDPsocket socket) {
	int error;
	
	if (socket == NULL) {
		error("Socket is NULL", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	// error = SDLNet_UDP_DelSocket(socketSet_g, socket);
	// if (error == -1) {
	// 	error("SDLNet_UDP_DelSocket returned %s", SDL_GetError());
	// 	// Not that we care... The socket will be freed either way.
	// }
	
	SDLNet_UDP_Close(socket);
	socket = NULL;
	
	error = 0;
	cleanup_l:
	
	return error;
}

int l_snetwork_send(const uint8_t *data, int length, IPaddress ipAddress) {
	int error;
	
	UDPpacket *packet = SDLNet_AllocPacket(length);
	if (packet == NULL) {
		error("SDLNet_AllocPacket returned %s", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	memcpy(packet->data, data, length);
	packet->len = length;
	
	packet->address = ipAddress;
	
	error = SDLNet_UDP_Send(g_serverSocket, -1, packet);
	
	if (error == 0) {
		error("SDLNet_UDP_Send returned %s", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = 0;
	cleanup_l:
	
	SDLNet_FreePacket(packet);
	
	return error;
}

int snetwork_init(void) {
	int error;
	
	IPaddress ipAddress;
	cfg_var_t *varServerPort;
	cfg_var_t *varClientPort;
	
	error = SDLNet_Init();
	if (error == -1) {
		critical_error("SDLNet_Init returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	varServerPort = cfg_findVar("server_port");
	if (varServerPort == NULL) {
		critical_error("\"server_port\" undefined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	g_serverSocket = SDLNet_UDP_Open(varServerPort->integer);
	if (g_serverSocket == NULL) {
		critical_error("SDLNet_UDP_Open returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	varClientPort = cfg_findVar("client_port");
	if (varClientPort == NULL) {
		critical_error("\"client_port\" undefined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = SDLNet_ResolveHost(&ipAddress, NULL, varClientPort->integer);
	if (error != 0) {
		critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	info("Opened UDP socket on port %i", varServerPort->integer);
	
	// socketSet_g = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	// if (socketSet_g == NULL) {
	// 	critical_error("SDLNet_AllocSocketSet returned %s", SDL_GetError());
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	// error = SDLNet_UDP_AddSocket(socketSet_g, g_serverSocket);
	// if (error == -1) {
	// 	critical_error("SDLNet_UDP_AddSocket returned %s", SDL_GetError());
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	error = 0;
	
	cleanup_l:
	
	return error;
}

void snetwork_quit(void) {

	// Don't care about errors.
	snetwork_closeSocket(g_serverSocket);
	
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients_g[i].socket == NULL) {
			continue;
		}
		// Don't care about errors.
		snetwork_closeSocket(clients_g[i].socket);
	}
	
	// SDLNet_FreeSocketSet(socketSet_g);
	// socketSet_g = NULL;
	
	SDLNet_Quit();
}
