#ifndef JSON_RPC_H
#define JSON_RPC_H

#include "json.h"
#include "opcda_client.h"
#include "encoding.h"
#include "json_rpc_msg.h"

class OPCDADriverServer {
private:
	uint16_t port;
	WSADATA wd;
	SOCKET s;
public:
	OPCDADriverServer(uint16_t port);
	int start();
	~OPCDADriverServer();
};

class OPCDADriverConnection {
private:
	SOCKET fd;

public:
	OPCDADriverConnection(SOCKET fd);
	~OPCDADriverConnection();
	int start();
};

#endif