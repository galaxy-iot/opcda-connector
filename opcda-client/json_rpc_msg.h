#ifndef JSON_RPC_MSG_H
#define JSON_RPC_MSG_H

#include <string>
#include <vector>
#include "opcda_client.h"
#include "json.h"

#define ConnectMsg    0
#define AddItemMsg    1
#define QueryMsg      2
#define WriteMsg      3
#define RefreshMsg    4
#define ACK           5
#define QueryAck      6
#define WriteValue    7

struct Msg {
	unsigned char command;
	unsigned int  length;
	char* payload;

	bool alloc;

	~Msg() {
		if (alloc) {
			free(payload);
		}
	}

	Msg() :alloc(false) {}

	Msg(unsigned char command, unsigned int  length, char* payload) {
		this->command = command;
		this->length = length;
		this->payload = payload;
	}
};

struct MsgAck {
	bool OK;
	std::string error;
};

int readMsg(int fd, Msg* msg);
int writeAck(int fd, bool ok, std::string error);


struct ConnectPayload {
	std::string ProgID;
	std::string Host;
};

int handleConnectMsg(Msg& msg, ConnectPayload& paylod);
int unmarshalConnectPayload(char* payload, ConnectPayload& paylod);


struct AddItemPayload {
	std::vector<std::string> items;
};

int handleAddItemMsg(Msg& msg, AddItemPayload& paylod);
int unmarshalAddItemPayload(char* payload, AddItemPayload& paylod);

int queryDataAck(int fd, OPCDAClient& c);

struct WriteDataPayload {
	std::string item;
	std::string value;
};

int handleWriteDataMsg(Msg& msg, WriteDataPayload& paylod);
int unmarshalWriteDataPayload(char* payload, WriteDataPayload& paylod);

void refreshInvalidItem(OPCDAClient& c);
#endif