#ifndef OPCDA_ASYNC_H
#define OPCDA_ASYNC_H

#include <atlbase.h>
#include <atlcom.h>
#include <iostream>
#include <unordered_map>

#include "opcda_item.h"
#include "opcda.h"

class CAsyncDataCallback : public IOPCDataCallback
{
private:
    DWORD ReferenceCount;
public:
    CAsyncDataCallback();
    virtual ~CAsyncDataCallback() {};
  
    STDMETHODIMP QueryInterface(REFIID iid, LPVOID* ppInterface);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP OnDataChange(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterQuality, HRESULT masterError,
        DWORD count, OPCHANDLE* clientHandles, VARIANT* values, WORD* quality, FILETIME* time,
        HRESULT* errors);

    STDMETHODIMP OnReadComplete(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterQuality, HRESULT masterError,
        DWORD count, OPCHANDLE* clientHandles, VARIANT* values, WORD* quality, FILETIME* time,
        HRESULT* errors);

    STDMETHODIMP OnWriteComplete(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterError, DWORD count,
        OPCHANDLE* clientHandles, HRESULT* errors);

    STDMETHODIMP OnCancelComplete(DWORD transactionID, OPCHANDLE groupHandle);
}; // CAsyncDataCallback


#endif