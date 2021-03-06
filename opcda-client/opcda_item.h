#ifndef OPCDA_ITEM_H
#define OPCDA_ITEM_H

#include <COMCat.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <string>
#include "opcda.h"

class OPCDAItem {
private:
    OPCHANDLE ServersItemHandle;
    VARTYPE VtCanonicalDataType;
    bool valid;
    VARIANT value;
    WORD quality;
    std::string ItemName;

public:
    OPCDAItem();
    OPCDAItem(std::string ItemName);

    void setValid(bool valid);
    void setDataType(VARTYPE type);
    void setServerItemHandle(OPCHANDLE ServersItemHandle);
    void setValue(VARIANT value);
    void setQuality(WORD quality);

    bool getValid();
    VARTYPE getDataType();
    OPCHANDLE getServerItemHandle();
    VARIANT getValue();
    WORD getQuality();

    ~OPCDAItem();
};

#endif