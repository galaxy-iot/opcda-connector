#include <iostream>
#include <windows.h>

#include "opcda_client.h"

int main() {
	OPCDAClient d;

	int ret = d.connectToServer(L"Matrikon.OPC.Simulation.1",L"192.168.123.197");
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


	ret = d.addItem("aaa.aaa", true);
	if (ret < 0) {
		std::cout << "add items failed" << std::endl;
		exit(-1);
	}
	d.addItem("aaa.aaa1", true);
	d.addItem("aaa.aaa2", true);

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

	return 0;
}