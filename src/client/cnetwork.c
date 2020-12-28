
#include "cnetwork.h"
#include "../common/log.h"
#include "../common/cfg.h"
#include "../common/insane.h"

UDPsocket serverSocket_g;
UDPsocket clientSocket_g;
// SDLNet_SocketSet socketSet_g;


int cnetwork_closeSocket(UDPsocket socket) {
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

int l_cnetwork_receive(Uint8 **data, int *length) {
	int error;
	
	UDPpacket *packet = SDLNet_AllocPacket(100);
	if (packet == NULL) {
		error("SDLNet_AllocPacket returned %s", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = SDLNet_UDP_Recv(clientSocket_g, packet);
	if (error < 0) {
		error("SDLNet_UDP_Recv returned %s", SDL_GetError());
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	if (error > 0) {
		// We got something!
		*data = realloc(*data, packet->len * sizeof(Uint8));
		memcpy(*data, packet->data, packet->len);
		*length = packet->len;
	}
	else {
		*length = 0;
		// insane_free(data);
	}
	
	error = 0;
	cleanup_l:
	
	SDLNet_FreePacket(packet);
	
	return error;
}

// int l_cnetwork_send(const uint8_t *data, int length, IPaddress ipAddress) {
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
	
// 	error = SDLNet_UDP_Send(serverSocket_g, -1, packet);
	
// 	if (error == 0) {
// 		error("SDLNet_UDP_Send returned %s", SDL_GetError());
// 		error = ERR_GENERIC;
// 		goto cleanup_l;
// 	}
	
// 	error = 0;
// 	cleanup_l:
	
// 	SDL_free(packet);
	
// 	return error;
// }

int cnetwork_init(void) {
	int error;
	
	IPaddress ipAddress;
	cfg_var_t *varClientPort;
	cfg_var_t *varServerPort;
	cfg_var_t *varIpAddress;
	
	error = SDLNet_Init();
	if (error == -1) {
		critical_error("SDLNet_Init returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	varClientPort = cfg_findVar("client_port");
	if (varClientPort == NULL) {
		critical_error("\"client_port\" undefined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	clientSocket_g = SDLNet_UDP_Open(varClientPort->integer);
	if (clientSocket_g == NULL) {
		critical_error("SDLNet_UDP_Open returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	varServerPort = cfg_findVar("server_port");
	if (varServerPort == NULL) {
		critical_error("\"server_port\" undefined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	varIpAddress = cfg_findVar("ip_address");
	if (varIpAddress == NULL) {
		critical_error("\"ip_address undefined", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	error = SDLNet_ResolveHost(&ipAddress, varIpAddress->string.value, varServerPort->integer);
	if (error != 0) {
		critical_error("SDLNet_ResolveHost returned %s", SDL_GetError());
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	info("Opened UDP socket on port %i", varClientPort->integer);
	
	// socketSet_g = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	// if (socketSet_g == NULL) {
	// 	critical_error("SDLNet_AllocSocketSet returned %s", SDL_GetError());
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	// error = SDLNet_UDP_AddSocket(socketSet_g, serverSocket_g);
	// if (error == -1) {
	// 	critical_error("SDLNet_UDP_AddSocket returned %s", SDL_GetError());
	// 	error = ERR_CRITICAL;
	// 	goto cleanup_l;
	// }
	
	error = 0;
	
	cleanup_l:
	
	return error;
}

void cnetwork_quit(void) {

	// Don't care about errors.
	cnetwork_closeSocket(clientSocket_g);
	
	// for (int i = 0; i < MAX_CLIENTS; i++) {
	// 	if (clients_g[i].socket == NULL) {
	// 		continue;
	// 	}
	// 	// Don't care about errors.
	// 	snetwork_closeSocket(clients_g[i].socket);
	// }
	
	// SDLNet_FreeSocketSet(socketSet_g);
	// socketSet_g = NULL;
	
	SDLNet_Quit();
}
