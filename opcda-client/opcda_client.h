#pragma once

#include <COMCat.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <atlexcept.h>
#include <atlstr.h>
#include <objbase.h>
#include <stdexcept>
#include <unordered_map>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "opccomn.h"
#include "opc_enum.h"
#include "opcda.h"
#include "opcda_item.h"

class OPCDAClient
{
private:
    DWORD GroupHandle;
    ATL::CComPtr<IOPCGroupStateMgt> iStateManagement;
    ATL::CComPtr<IOPCSyncIO> iSyncIO;
    ATL::CComPtr<IOPCAsyncIO2> iAsync2IO;
    ATL::CComPtr<IOPCItemMgt> iItemManagement;
    
    ATL::CComPtr<IOPCServer> iOpcServer;
    ATL::CComPtr<IUnknown> iUnknown;

    std::unordered_map<std::wstring, OPCDAItem> itemMap;
public:
    OPCDAClient();
    ~OPCDAClient();

    int connectToServer(const std::wstring& progId, const std::wstring& server);
    int newGroup(const std::wstring &groupName,bool active, unsigned long reqUpdateRate_ms,
        unsigned long& revisedUpdateRate_ms, float deadBand);

    int addItem(std::wstring itemName, bool active);
    int addItems(std::vector<std::wstring> &itemNames, bool active);

    int readItem(std::wstring itemName);
    int getItemValue(std::wstring itemName);
}; 