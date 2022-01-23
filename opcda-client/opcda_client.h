#pragma once

#ifndef OPCDA_CLIENT_H
#define OPCDA_CLIENT_H

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
#include <string>
#include <comutil.h>

#include "opccomn.h"
#include "opc_enum.h"
#include "opcda.h"
#include "opcda_item.h"
#include "opcda_async.h"
#include "encoding.h"

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

    std::unordered_map<std::string, OPCDAItem> itemMap;

    ATL::CComPtr<CAsyncDataCallback> AsyncDataCallBackHandler;
    ATL::CComPtr<IConnectionPoint> iAsyncDataCallbackConnectionPoint;

    DWORD GroupCallbackHandle;
    ATL::CComPtr<IMalloc> iMalloc;
public:
    OPCDAClient();
    ~OPCDAClient();

    int connectToServer(const std::wstring& progId, const std::wstring& server);
    HRESULT getListOfDAServersEx(std::wstring hostName, tagCLSCTX serverLocation, CATID cid,
        std::vector<std::wstring>& listOfProgIDs, std::vector<CLSID>& listOfClassIDs);
    int newGroup(const std::wstring &groupName,bool active, unsigned long reqUpdateRate_ms,
        unsigned long& revisedUpdateRate_ms, float deadBand);

    int addItem(std::string itemName, bool active);
    int addItems(std::vector<std::string> &itemNames, bool active);
    int removeItem(std::string itemName);

    int readItem(std::string itemName);
    void printValues();

    int enableAsync();
    int refresh();

    std::unordered_map<std::string, OPCDAItem>& getDataMap(); 
    static std::string VariantToString(VARTYPE type, VARIANT data);
    static int OPCDAClient::StringToVariant(VARTYPE type, std::string value, VARIANT& va);

    int writeValue(std::string item,std::string value);

    void OPCDAClient::comFree(void* memory);
    void comFreeVariant(VARIANT* memory, unsigned size);
}; 

#endif