// opcda-client.cpp: 定义应用程序的入口点。
//

#include "opcda_client.h"


OPCDAClient::OPCDAClient() {
    HRESULT result = CoInitialize(nullptr);
    if (FAILED(result))
    {
        
    }

    CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr,
        EOAC_NONE, nullptr);
}

OPCDAClient::~OPCDAClient() {
    iOpcServer = nullptr;
    iUnknown = nullptr;
    iStateManagement = nullptr;
    iSyncIO = nullptr;
    iAsync2IO = nullptr;
    iItemManagement = nullptr;

    printf("%s\n","dconstruct");
    CoUninitialize();
}

HRESULT makeCOMObjectEx(std::wstring hostName, tagCLSCTX serverLocation, const IID requestedClass,
    const IID requestedInterface, void** interfacePtr)
{
    COAUTHINFO authn = { 0 };
    // Set up the NULL security information
    authn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT; // RPC_C_AUTHN_LEVEL_NONE
    authn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    authn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    authn.dwCapabilities = EOAC_NONE;
    authn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    authn.pAuthIdentityData = nullptr;
    authn.pwszServerPrincName = nullptr;

    COSERVERINFO requestedServerInfo = { 0 };
    CW2W wstr(hostName.c_str());
    requestedServerInfo.pwszName = wstr;
    requestedServerInfo.pAuthInfo = &authn;

    MULTI_QI reqInterface;
    reqInterface.pIID = &requestedInterface;
    reqInterface.pItf = nullptr;
    reqInterface.hr = S_OK;

    HRESULT result =
        CoCreateInstanceEx(requestedClass, nullptr, serverLocation, &requestedServerInfo, 1, &reqInterface);
    if (FAILED(result))
    {
        return result;
    } // if

    *interfacePtr = reqInterface.pItf; // avoid ref counter getting incremented again
    return S_OK;
} // COPCHost::makeCOMObjectEx


HRESULT getListOfDAServersEx(std::wstring hostName, tagCLSCTX serverLocation, CATID cid,
    std::vector<std::wstring>& listOfProgIDs, std::vector<CLSID>& listOfClassIDs)
{
    CATID implist[1] = { cid };
    ATL::CComPtr<IEnumCLSID> iEnum;
    ATL::CComPtr<IOPCServerList> iCatInfo;

    HRESULT ret = makeCOMObjectEx(hostName, serverLocation, CLSID_OpcServerList, IID_IOPCServerList, (void**)&iCatInfo);
    if (FAILED(ret))
    {
        return ret;
    }

    ret = iCatInfo->EnumClassesOfCategories(1, implist, 0, nullptr, &iEnum);
    if (FAILED(ret))
    {
        return ret;
    }

    GUID classID = { 0, 0, 0, {0} };
    ULONG actual = 0;
    while ((ret = iEnum->Next(1, &classID, &actual)) == S_OK)
    {
        LPOLESTR progID = nullptr;
        LPOLESTR userType = nullptr;
        ret = iCatInfo->GetClassDetails(classID, &progID, &userType); // ProgIDFromCLSID ( classID, &progID )
        if (FAILED(ret))
        {
            return ret;
        }
        else
        {
            listOfClassIDs.push_back(classID);
            listOfProgIDs.push_back(progID);

            LPOLESTR classIDStr = nullptr;
            ret = StringFromCLSID(classID, &classIDStr);
            if (FAILED(ret))
            {
                return ret;
            }
        } // else
    }     // while

} // COPCHost::getListOfDAServersEx


int OPCDAClient::connectToServer(const std::wstring& progId, const std::wstring& server) {
    std::vector<CLSID> localClassIdList;
    std::vector<std::wstring> localServerList;
    HRESULT ret = getListOfDAServersEx(server, CLSCTX_REMOTE_SERVER,IID_CATID_OPCDAServer20, localServerList, localClassIdList);
    if (FAILED(ret))
    {
        return -1;
    }

    bool foundProg = false;
    CLSID clsid;
    for (size_t i = 0; i < localServerList.size(); i++)
    {
        if (localServerList.at(i) == progId)
        {
            foundProg = true;
            clsid = localClassIdList.at(i);
            break;
        }
    }

    if (!foundProg) {
        return -1;
    }

    
    HRESULT result;
    result = makeCOMObjectEx(server, CLSCTX_REMOTE_SERVER, clsid, IID_IUnknown, (void **)&iUnknown);
    if (FAILED(result))
    {
        return -1;
    }

    result = iUnknown->QueryInterface(IID_IOPCServer, (void **)&iOpcServer);
    if (FAILED(result))
    {
        return -1;
    }

    return 0;
}


int OPCDAClient::newGroup(const std::wstring& groupName, bool active, unsigned long reqUpdateRate_ms,
    unsigned long& revisedUpdateRate_ms, float deadBand) {

    HRESULT result = iOpcServer->AddGroup(groupName.c_str(), active, reqUpdateRate_ms, NULL, NULL,
        &deadBand, 0, &GroupHandle, &revisedUpdateRate_ms,
        IID_IOPCGroupStateMgt, (LPUNKNOWN*)&iStateManagement);
    if (FAILED(result))
    {
        return -1;
    }

    std::cout << GroupHandle << std::endl;

    result = iStateManagement->QueryInterface(IID_IOPCSyncIO, (void**)&iSyncIO);
    if (FAILED(result))
    {
        return -1;
    }

    result = iStateManagement->QueryInterface(IID_IOPCAsyncIO2, (void**)&iAsync2IO);
    if (FAILED(result))
    {
        return -1;
    }

    
    result = iStateManagement->QueryInterface(IID_IOPCItemMgt, (void**)&iItemManagement);
    if (FAILED(result))
    {
        return -1;
    }
    
    return 0;
}

int OPCDAClient::addItem(std::wstring itemName,bool active) {
    std::vector<std::wstring> v;
    v.push_back(itemName);

    return this->addItems(v,active);
}

int OPCDAClient::addItems(std::vector<std::wstring>& itemNames, bool active) {
    OPCITEMDEF* m_Items = new OPCITEMDEF[itemNames.size()];

    for (int i = 0; i < itemNames.size(); i++) {
        if (this->itemMap.find(itemNames[i]) == this->itemMap.end()) {
            itemMap.insert(std::pair<std::wstring, OPCDAItem>(itemNames[i], OPCDAItem(itemNames[i])));
        }

        m_Items[i].szItemID = (wchar_t*)(itemNames[i].c_str());
        m_Items[i].bActive = active;
        m_Items[i].hClient = (OPCHANDLE) & (itemMap[itemNames[i]]);
        m_Items[i].dwBlobSize = 0;
        m_Items[i].pBlob = nullptr;
        m_Items[i].vtRequestedDataType = VT_EMPTY;
        m_Items[i].szAccessPath = L"";
    }

    HRESULT* results = nullptr;
    OPCITEMRESULT* details = nullptr;

    HRESULT result = iItemManagement->AddItems(itemNames.size(), m_Items, &details, &results);
    delete(m_Items);
    if (FAILED(result)) {
        return -1;
    }

    for (int i = 0; i < itemNames.size(); i++) {
        if (details[i].pBlob)
        {
            CoTaskMemFree(details[i].pBlob);
        }

        if (FAILED(results[i])) {
            std::wcout << "item " << itemNames[i] << " is invalid" << std::endl;
            this->itemMap[itemNames[i]].setValid(false);
        }
        else {
            std::wcout << "item " << itemNames[i] << " is valid" << std::endl;
            this->itemMap[itemNames[i]].setValid(true);
            this->itemMap[itemNames[i]].setDataType(details[i].vtCanonicalDataType);
            this->itemMap[itemNames[i]].setServerItemHandle(details[i].hServer);
        }
    }

    CoTaskMemFree(details);
    CoTaskMemFree(results);

    return 0;
}

int OPCDAClient::readItem(std::wstring itemName) {
    if (this->itemMap.find(itemName) == this->itemMap.end()) {
        return -1;
    }
   
    OPCHANDLE phServer[1];
    OPCITEMSTATE* pItemValue = nullptr;
    HRESULT* pErrors = nullptr;

    phServer[0] = itemMap[itemName].getServerItemHandle();
    
    HRESULT ret = iSyncIO->Read(OPC_DS_CACHE, 1, phServer, &pItemValue, &pErrors);
    if (FAILED(ret)) {
        return -1;
    }

    itemMap[itemName].setQuality(pItemValue[0].wQuality);
    itemMap[itemName].setValue(pItemValue[0].vDataValue);

    CoTaskMemFree(pItemValue);
    CoTaskMemFree(pErrors);

    return 0;
}

int OPCDAClient::getItemValue(std::wstring itemName) {
    if (this->itemMap.find(itemName) == this->itemMap.end()) {
        std::wcout << "item " << itemName << " is not in group" << std::endl;
        return -1;
    }

    std::cout << itemMap[itemName].getDataType() << std::endl;
}