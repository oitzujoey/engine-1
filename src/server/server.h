
#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <enet/enet.h>
#include "../common/common.h"

typedef struct {
	bool inUse;
	bool disconnectPending;
	ENetPeer *peer;
} client_t;

typedef struct {
	ENetAddress ipAddress;
	ENetHost *host;
	int maxClients;
	int connectedClients;
} serverState_t;

#endif
