#ifndef ENCODING_H
#define ENCODING_H

#include <codecvt>

std::string WS2S(const std::wstring& wstr);

std::wstring S2WS(const std::string& str);

#endif