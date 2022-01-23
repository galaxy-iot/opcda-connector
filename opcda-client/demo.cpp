#include <iostream>
#include <windows.h>

#include "opcda_client.h"
#include "encoding.h"

int main() {
	std::string ret1;
	std::string newItemName = "BZZ-CKCW-SD";
	for (int i = 0; i < newItemName.size(); i++) {
		ret1 += "\\x" + std::to_string(newItemName[i]);
	}

	std::cout << ret1 << std::endl;

	OPCDAClient d;

	int ret = d.connectToServer(L"OPCServer.WinCC.1",L"10.30.2.13");
	if (ret < 0) {
		std::cout << "connect to server failed" << std::endl;
		exit(-1);
	}
	
	std::cout << "connect to server successfully" << std::endl;

	
	unsigned long refreshRate;
	ret = d.newGroup(L"DemoGroup", true, 1000, refreshRate, 0.0);
	if (ret < 0) {
		std::cout << "new group failed" << std::endl;
		exit(-1);
	}

	std::cout << "create group successfully" << std::endl;


	ret = d.addItem("B料位计B下限_1", true);
	if (ret < 0) {
		std::cout << "add items failed" << std::endl;
		exit(-1);
	}

	ret = d.readItem("B料位计B下限_1");
	if (ret < 0) {
		std::cout << "read items failed" << std::endl;
		exit(-1);
	}

	d.printValues();
	/*
	ret = d.writeValue("aaa.aaaa","shiq");
	if (ret < 0) {
		std::cout << "write failed" << std::endl;
		exit(-1);
	}

	ret = d.enableAsync();
	if (ret < 0) {
		std::cout << "enable async failed" << std::endl;
		exit(-1);
	}

	std::cout << "enable async successfully" << std::endl;

	while (true) {
		d.refresh();
		Sleep(1000);
		d.printValues();
	}
	*/
	return 0;
}