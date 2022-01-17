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
    AsyncDataCallBackHandler = nullptr;
    iAsyncDataCallbackConnectionPoint = nullptr;

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

            CoTaskMemFree(progID);
            CoTaskMemFree(userType);
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

int OPCDAClient::removeItem(std::string itemName) {
    if (this->itemMap.find(itemName) == this->itemMap.end()) {
        return 0;
    }
    
    OPCHANDLE handlers[1];
    handlers[0] = itemMap[itemName].getServerItemHandle();
    HRESULT* results = nullptr;

    HRESULT result = iItemManagement->RemoveItems(1, handlers,&results);
    if (FAILED(result)) {
        return -1;
    }

    if (FAILED(results[0])) {
        return -1;
    }

    CoTaskMemFree(results);
    return 0;
}

int OPCDAClient::addItem(std::string itemName,bool active) {
    OPCITEMDEF m_Items[1];

    if (this->itemMap.find(itemName) == this->itemMap.end()) {
        std::cout << itemName << " not exist" << std::endl;
        itemMap.insert(std::pair<std::string, OPCDAItem>(itemName, OPCDAItem(itemName)));
    }
    else {
        std::cout << removeItem(itemName) << std::endl;
    }

    m_Items[0].szItemID = (wchar_t*)(S2WS(itemName).c_str());
    m_Items[0].bActive = active;
    m_Items[0].hClient = (OPCHANDLE) & (itemMap[itemName]);
    m_Items[0].dwBlobSize = 0;
    m_Items[0].pBlob = nullptr;
    m_Items[0].vtRequestedDataType = VT_EMPTY;
    m_Items[0].szAccessPath = L"";

    HRESULT* results = nullptr;
    OPCITEMRESULT* details = nullptr;

    HRESULT result = iItemManagement->AddItems(1, m_Items, &details, &results);
    if (FAILED(result)) {
        this->itemMap[itemName].setValid(false);
        return 0;
    }

    if (details[0].pBlob)
    {
        CoTaskMemFree(details[0].pBlob);
    }

    if (FAILED(results[0])) {
        this->itemMap[itemName].setValid(false);
    }
    else {
        this->itemMap[itemName].setValid(true);
        this->itemMap[itemName].setDataType(details[0].vtCanonicalDataType);
        this->itemMap[itemName].setServerItemHandle(details[0].hServer);
    }

    CoTaskMemFree(details);
    CoTaskMemFree(results);

    return 0;
}

int OPCDAClient::addItems(std::vector<std::string>& itemNames, bool active) {
    for (int i = 0; i < itemNames.size(); i++) {
        addItem(itemNames[i],active);
    }

    return 0;
}

int OPCDAClient::readItem(std::string itemName) {
    if (this->itemMap.find(itemName) == this->itemMap.end()) {
        return -1;
    }
   
    OPCHANDLE phServer[1];
    OPCITEMSTATE* pItemValue = nullptr;
    HRESULT* pErrors = nullptr;

    phServer[0] = itemMap[itemName].getServerItemHandle();
    
    HRESULT ret = iSyncIO->Read(OPC_DS_DEVICE, 1, phServer, &pItemValue, &pErrors);
    if (FAILED(ret)) {
        return -1;
    }

    itemMap[itemName].setQuality(pItemValue[0].wQuality);
    itemMap[itemName].setValue(pItemValue[0].vDataValue);

    CoTaskMemFree(pItemValue);
    CoTaskMemFree(pErrors);

    return 0;
}

int OPCDAClient::enableAsync() {
    ATL::CComPtr<IConnectionPointContainer> iConnectionPointContainer = 0;
    HRESULT result =
        iStateManagement->QueryInterface(IID_IConnectionPointContainer, (void**)&iConnectionPointContainer);
    if (FAILED(result))
    {
        return -1;
    }

    result = iConnectionPointContainer->FindConnectionPoint(IID_IOPCDataCallback, &iAsyncDataCallbackConnectionPoint);
    if (FAILED(result))
    {
        return -1;
    }

    AsyncDataCallBackHandler = new CAsyncDataCallback();
    result = iAsyncDataCallbackConnectionPoint->Advise(AsyncDataCallBackHandler, &GroupCallbackHandle);
    if (FAILED(result))
    {
        iAsyncDataCallbackConnectionPoint = nullptr;
        AsyncDataCallBackHandler = nullptr;
        return -1;
    }

    return 0;
}

int OPCDAClient::refresh() {
    DWORD cancelID = 0;

    HRESULT result = iAsync2IO->Refresh2(OPC_DS_DEVICE, 1, &cancelID);
    if (FAILED(result))
    {
        return 1;
    } // if
    return 0;
}

std::unordered_map<std::string, OPCDAItem>& OPCDAClient::getDataMap() {
    return this->itemMap;
}

void OPCDAClient::printValues() {
    for (auto iter = itemMap.begin(); iter != itemMap.end(); iter++)
    {
        std::cout << "item:" << iter->first << " ok:" << iter->second.getValid() << ", value:" <<
            VariantToString(iter->second.getDataType(),iter->second.getValue()) << std::endl;
    }
}

std::string OPCDAClient::VariantToString(VARTYPE type,VARIANT data)
{
    std::string strValue;
    TCHAR szValue[1024] = { 0x00 };

    switch (type)
    {
    case VT_BSTR:
    case VT_LPSTR:
    case VT_LPWSTR:
        strValue = (LPCTSTR)data.bstrVal;
        break;

    case VT_I1:
    case VT_UI1:
        sprintf_s(szValue, _T("%d"), data.bVal);
        strValue = szValue;
        break;

    case VT_I2:
        sprintf_s(szValue, _T("%d"), data.iVal);
        strValue = szValue;
        break;

    case VT_UI2:
        sprintf_s(szValue, _T("%d"), data.uiVal);
        strValue = szValue;
        break;

    case VT_INT:
        sprintf_s(szValue, _T("%d"), data.intVal);
        strValue = szValue;
        break;

    case VT_I4:
        sprintf_s(szValue, _T("%d"), data.lVal);
        strValue = szValue;
        break;

    case VT_I8:
        sprintf_s(szValue, _T("%ld"), data.bVal);
        strValue = szValue;
        break;

    case VT_UINT:
        sprintf_s(szValue, _T("%u"), data.uintVal);
        strValue = szValue;
        break;

    case VT_UI4:
        sprintf_s(szValue, _T("%u"), data.ulVal);
        strValue = szValue;
        break;

    case VT_UI8:
        sprintf_s(szValue, _T("%u"), data.ulVal);
        strValue = szValue;
        break;

    case VT_VOID:
        sprintf_s(szValue, _T("%8x"), (unsigned int)data.byref);
        strValue = szValue;
        break;

    case VT_R4:
        sprintf_s(szValue, _T("%.4f"), data.fltVal);
        strValue = szValue;
        break;

    case VT_R8:
        sprintf_s(szValue, _T("%.8f"), data.dblVal);
        strValue = szValue;
        break;

    case VT_DECIMAL:
        sprintf_s(szValue, _T("%.4f"), data.decVal);
        strValue = szValue;
        break;

    case VT_CY:
        //COleCurrency cy = data.vDataValue.cyVal;
        //strValue = cy.Format();
        break;

    case VT_BOOL:
        strValue = data.boolVal ? "TRUE" : "FALSE";
        break;

    case VT_DATE: {
        DATE dt = data.date;
        // COleDateTime da = COleDateTime(dt);
        // strValue = da.Format("%Y-%m-%d %H:%M:%S");
        break;
    }


    case VT_NULL:
        strValue = "VT_NULL";
        break;

    case VT_EMPTY:
        strValue = "";
        break;

    case VT_UNKNOWN:
    default:
        strValue = "UN_KNOWN";
        break;
    }

    return strValue;
}
