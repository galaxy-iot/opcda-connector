#include "opcda_async.h"

CAsyncDataCallback::CAsyncDataCallback() : ReferenceCount(0) {}

STDMETHODIMP CAsyncDataCallback::QueryInterface(REFIID iid, LPVOID* ppInterface)
{
    if (!ppInterface)
    {
        return E_INVALIDARG;
    }

    if (iid == IID_IUnknown)
    {
        *ppInterface = (IUnknown*)this;
    }
    else if (iid == IID_IOPCDataCallback)
    {
        *ppInterface = (IOPCDataCallback*)this;
    }
    else
    {
        *ppInterface = nullptr;
        return E_NOINTERFACE;
    } // else

    AddRef();
    return S_OK;
} // QueryInterface

STDMETHODIMP_(ULONG) CAsyncDataCallback::AddRef()
{
    return ++ReferenceCount;
} // AddRef


STDMETHODIMP_(ULONG) CAsyncDataCallback::Release()
{
    DWORD count = ReferenceCount ? --ReferenceCount : 0;

    if (!count)
    {
        delete this;
    }

    return count;
} // Release

STDMETHODIMP CAsyncDataCallback::OnDataChange(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterQuality, HRESULT masterError,
    DWORD count, OPCHANDLE* clientHandles, VARIANT* values, WORD* quality, FILETIME* time,
    HRESULT* errors)
{
    std::cout << "data changed:" << count << std::endl;

    OPCDAItem* item;
    for (int i = 0; i < count; i++) {
        item = (OPCDAItem*)clientHandles[i];
        
        item->setQuality(quality[i]);
        item->setValue(values[i]);
    }

    return S_OK;
} // OnDataChange

STDMETHODIMP CAsyncDataCallback::OnReadComplete(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterQuality, HRESULT masterError,
    DWORD count, OPCHANDLE* clientHandles, VARIANT* values, WORD* quality, FILETIME* time,
    HRESULT* errors)
{
    std::cout << "on read complete" << std::endl;
    return S_OK;
} // OnReadComplete

STDMETHODIMP CAsyncDataCallback::OnWriteComplete(DWORD transactionID, OPCHANDLE groupHandle, HRESULT masterError, DWORD count,
    OPCHANDLE* clientHandles, HRESULT* errors)
{

    return S_OK;
} // OnWriteComplete

STDMETHODIMP CAsyncDataCallback::OnCancelComplete(DWORD transactionID, OPCHANDLE groupHandle)
{
    printf("OnCancelComplete: transactionID=%ld groupHandle=%ld\n", transactionID, groupHandle);
    return S_OK;
} // OnCancelComplete