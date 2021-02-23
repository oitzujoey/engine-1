
#ifndef CNETWORK_H
#define CNETWORK_H

#include "../common/types.h"

// extern UDPsocket g_clientSocket;

// int cnetwork_closeSocket(UDPsocket socket);
// int l_cnetwork_receive(Uint8 **data, int *length);

typedef enum cnetwork_connectionState_e {
	cnetwork_connectionState_initial,
	cnetwork_connectionState_connecting,
	cnetwork_connectionState_connected,
	cnetwork_connectionState_disconnected
} cnetwork_connectionState_t;

typedef struct cnetwork_clientState_s {
	bool connected;
	ENetAddress serverAddress;
	const char *serverAddressString;
	cnetwork_connectionState_t connectionState;
	time_t lastResponseTime;
} cnetwork_clientState_t;

/* Config events */
int cnetwork_callback_setIpAddress(cfg2_var_t *var, const char *command);
int cnetwork_callback_setServerPort(cfg2_var_t *var, const char *command);

/* Network functions */
int cnetwork_runEvents(void);
int cnetwork_init(void);
void cnetwork_quit(void);

#endif
