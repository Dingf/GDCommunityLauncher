#ifndef INC_GDCL_STRING_CONVERT_H
#define INC_GDCL_STRING_CONVERT_H

#include <string>

std::wstring RawToWide(const std::string& str);
std::string  WideToRaw(const std::wstring& str);
std::wstring CharToWide(const std::string& str);
std::string  WideToChar(const std::wstring& str);

#endif//INC_GDCL_STRING_CONVERT_H