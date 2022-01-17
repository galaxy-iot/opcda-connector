#include "json_rpc_msg.h"

int readN(int fd, char *buf,int length) {
	while (length > 0) {
		char ch;
		if (recv(fd, &ch, 1, 0) < 0) {
			return -1;
		}
		*buf++ = ch;
		length--;
	}

	return 0;
}

int readMsg(int fd, Msg* msg) {
	/*
	char buf[1024 * 32] = {0};
	if (recv(fd, buf, 1024 * 32, 0) < 0) {
		return -1;
	}

	msg->command = buf[0];
	msg->length = ntohl((buf[4] << 24) + (buf[3] << 16) + (buf[2] << 8) + buf[1]);

	printf("%d:%d\n", msg->command, msg->length);

	msg->payload = buf + 5;
	return 0;*/

	char buf[5] = {0};

	if (readN(fd, buf, 5) < 0) {
		return -1;
	}

	msg->command = buf[0];
	msg->length = ntohl((buf[4] << 24) + (buf[3] << 16) + (buf[2] << 8) + buf[1]);

	if (msg->length <= 0) {
		return 0;
	}

	char* buf1 = (char*)malloc(msg->length);
	if (readN(fd, buf1, msg->length) < 0) {
		free(buf1);
		return -1;
	}

	msg->alloc = true;
	msg->payload = buf1;
	return 0;
}

int writeAck(int fd, bool ok, std::string error) {
	picojson::value json;
	json.set<picojson::object>(picojson::object());
	json.get<picojson::object>()["ok"] = picojson::value(bool(ok));
	json.get<picojson::object>()["err"] = picojson::value(error);

	std::string ret = json.serialize();
	return send(fd, ret.c_str(), ret.size(), 0);
}

int handleConnectMsg(Msg& msg, ConnectPayload& paylod) {
	if (msg.command != ConnectMsg) {
		return -1;
	}

	return unmarshalConnectPayload(msg.payload, paylod);
}

int unmarshalConnectPayload(char* payload, ConnectPayload& connectPayload) {
	picojson::value val;
	std::string err = picojson::parse(val, payload);
	if (!err.empty())
	{
		return -1;
	}

	connectPayload.ProgID = val.get<picojson::object>()["progId"].get<std::string>();
	connectPayload.Host = val.get<picojson::object>()["host"].get<std::string>();

	return 0;
}


int handleAddItemMsg(Msg& msg, AddItemPayload& paylod) {
	if (msg.command != AddItemMsg) {
		return -1;
	}

	return unmarshalAddItemPayload(msg.payload, paylod);
}

int unmarshalAddItemPayload(char* payload, AddItemPayload& items) {
	picojson::value val;
	std::string err = picojson::parse(val, payload);
	if (!err.empty())
	{
		return -1;
	}

	picojson::array itemValue = val.get<picojson::object>()["items"].get<picojson::array>();

	std::cout << itemValue.size() << std::endl;

	for (int i = 0; i < itemValue.size(); i++)
	{
		std::string item = itemValue[i].get<std::string>();
		items.items.push_back(item);

		std::cout << item << std::endl;
	}

	return 0;
}

int queryDataAck(int fd, OPCDAClient& c) {
	auto itemMap = c.getDataMap();

	picojson::value v;
	v.set<picojson::object>(picojson::object());
	for (auto iter = itemMap.begin(); iter != itemMap.end(); iter++)
	{
		picojson::value retValue;
		retValue.set<picojson::object>(picojson::object());
		retValue.get<picojson::object>()["ok"] = picojson::value(iter->second.getValid());
		retValue.get<picojson::object>()["type"] = picojson::value(double(iter->second.getDataType()));

		if (iter->second.getQuality() != 192) {
			retValue.get<picojson::object>()["error"] = picojson::value("bad value");
		}
		else {
			retValue.get<picojson::object>()["val"] = picojson::value(OPCDAClient::VariantToString(iter->second.getDataType(), iter->second.getValue()));
		}

		v.get<picojson::object>()[iter->first] = retValue;
	}

	std::string s = v.serialize();

	unsigned long size = ntohl(s.size());

	uint8_t l1 = (size >> 24) & 0xFF;
	uint8_t l2 = (size >> 16) & 0xFF;
	uint8_t l3 = (size >> 8) & 0xFF;
	uint8_t l4 = (size >> 0) & 0xFF;

	char buf[5] = { QueryAck ,l4,l3,l2,l1 };

	if (send(fd, buf, 5, 0) < 0) {
		return -1;
	}

	return send(fd, s.c_str(), s.size(), 0);
}
