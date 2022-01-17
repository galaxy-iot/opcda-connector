#include "opcda_item.h"

OPCDAItem::OPCDAItem(std::string ItemName) : ItemName(ItemName) {}

OPCDAItem::~OPCDAItem() {}

OPCDAItem::OPCDAItem() {}

void OPCDAItem::setValid(bool valid)  {
	this->valid = valid;
}

bool OPCDAItem::getValid() {
	return this->valid;
}

void OPCDAItem::setDataType(VARTYPE type) {
	this->VtCanonicalDataType = type;
}

VARTYPE OPCDAItem::getDataType() {
	return this->VtCanonicalDataType;
}

void OPCDAItem::setServerItemHandle(OPCHANDLE ServersItemHandle) {
	this->ServersItemHandle = ServersItemHandle;
}

OPCHANDLE OPCDAItem::getServerItemHandle() {
	return this->ServersItemHandle;
}

void OPCDAItem::setValue(VARIANT value) {
	this->value = value;
}

VARIANT OPCDAItem::getValue() {
	return this->value;
}

void OPCDAItem::setQuality(WORD quality) {
	this->quality = quality;
}

WORD OPCDAItem::getQuality() {
	return this->quality;
}