#ifndef INC_GDCL_STRING_CONVERT_H
#define INC_GDCL_STRING_CONVERT_H

#include <string>
#include <vector>

std::wstring RawToWide(const std::string& str);
std::string  WideToRaw(const std::wstring& str);
std::wstring CharToWide(const std::string& str);
std::string  WideToChar(const std::wstring& str);
std::string BinaryToBase64(const std::vector<uint8_t>& str);
std::vector<uint8_t> Base64ToBinary(const std::string& str);

#endif//INC_GDCL_STRING_CONVERT_H