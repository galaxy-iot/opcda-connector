#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#include "opcda_client.h"
#include "json_rpc.h"
#include "encoding.h"

OPCDADriverServer::OPCDADriverServer(uint16_t port) : port(port) {}
OPCDADriverServer::~OPCDADriverServer() {
	closesocket(s);
	WSACleanup();
}

DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);

int OPCDADriverServer::start() {
	if (0 != WSAStartup(MAKEWORD(2, 2), &wd)) {
		std::cout << "WSAStartup error : " << GetLastError() << std::endl;
		return 0;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s) {
		std::cout << "socket error : " << GetLastError() << std::endl;
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8891);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	int len = sizeof(sockaddr_in);
	if (SOCKET_ERROR == bind(s, (SOCKADDR*)&addr, len)) {
		std::cout << "bind error : " << GetLastError() << std::endl;
		return 0;
	}

	if (listen(s, 1024) < 0) {
		std::cout << "listen error : " << GetLastError() << std::endl;
		return 0;
	}

	while (true) {
		sockaddr_in addrClient;
		len = sizeof(sockaddr_in);

		SOCKET c = accept(s, (SOCKADDR*)&addrClient, &len);
		if (INVALID_SOCKET != c) {
			HANDLE hThread = CreateThread(NULL, 0, ThreadFun, (LPVOID)c, 0, NULL);
			CloseHandle(hThread);
		}
	}
}

OPCDADriverConnection::OPCDADriverConnection(SOCKET fd) {
	
}

OPCDADriverConnection::~OPCDADriverConnection() {
	closesocket(fd);
}

int OPCDADriverConnection::start() {
	OPCDAClient c;

	Msg connectMsg;
	int ret = readMsg(fd, &connectMsg);
	if (ret < 0) {
		std::cout << "read connect msg failed" << std::endl;
		return -1;
	}
	ConnectPayload p;

	ret = handleConnectMsg(connectMsg, p);
	if (ret < 0) {
		std::cout << "failed to handle connect msg" << std::endl;
		return -1;
	}

	if (c.connectToServer(S2WS(p.ProgID), S2WS(p.Host)) < 0) {
		std::cout << "connect opcda server failed" << std::endl;
		return -1;
	}

	unsigned long freshRate;
	if (c.newGroup(L"readGroup", true, 1000, freshRate, 0.0) < 0) {
		std::cout << "create opcda group failed" << std::endl;
		return -1;
	}

	if (c.enableAsync() < 0) {
		std::cout << "enable async failed" << std::endl;
		return -1;
	}

	writeAck(fd,true,"");

	while (true)
	{
		Msg connectMsg;
		ret = readMsg(fd, &connectMsg);
		if (ret < 0) {
			goto exit;
		}

		//printf("read msg: %d\n", connectMsg.command);

		refreshInvalidItem(c);

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))                                                            \
		{                                                                                                              \
			TranslateMessage(&msg);                                                                                    \
			DispatchMessage(&msg);                                                                                     \
		}

		switch (connectMsg.command) {
		case ConnectMsg: {
			ConnectPayload p;
			ret = handleConnectMsg(connectMsg, p);
			if (ret < 0) {
				goto exit;
			}
			break;
		}

		case AddItemMsg: {
			AddItemPayload p;
			ret = handleAddItemMsg(connectMsg, p);
			if (ret < 0) {
				goto exit;
			}

			ret = c.addItems(p.items, true);
			if (ret < 0) {
				goto exit;
			}

			writeAck(fd, true, "");
			break;
		}
		case RefreshMsg: {
			/*
			ret = c.refresh();
			if (ret < 0) {
				goto exit;
			}
			*/
			break;
		}
		case QueryMsg: {
			queryDataAck(fd, c);
			break;
		}
		case WriteValue: {
			WriteDataPayload p;
			ret = handleWriteDataMsg(connectMsg, p);
			if (ret < 0) {
				goto exit;
			}

			c.writeValue(p.item, p.value);
		}
		}
	}

exit:
	return 0;
}

DWORD WINAPI ThreadFun(LPVOID lpThreadParameter) {
	SOCKET fd = (SOCKET)lpThreadParameter;

	std::cout << fd << std::endl;

	OPCDADriverConnection c(fd);
	c.start();

	return 0;
}


