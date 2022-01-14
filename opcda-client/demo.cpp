#include <iostream>
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

	std::vector<std::wstring> v;
	v.push_back(L"aaa.aaa");
	v.push_back(L"aaa.aaa1aaa");

	ret = d.addItems(v,true);
	if (ret < 0) {
		std::cout << "add items failed" << std::endl;
		exit(-1);
	}

	ret = d.readItem(L"aaa.aaa");
	if (ret < 0) {
		std::cout << "read item failed" << std::endl;
		exit(-1);
	}

	d.getItemValue(L"aaa.aaa");

	return 0;
}