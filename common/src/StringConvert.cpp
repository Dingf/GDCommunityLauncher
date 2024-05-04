#include "StringConvert.h"

std::wstring RawToWide(const std::string& str)
{
    std::wstring result;
    result.reserve(str.size());
    for (size_t i = 0; (i + 1) < str.size(); i += 2)
    {
        result.push_back((((wchar_t)str[i]) << 8) | ((wchar_t)str[i+1]));
    }
    return result;
}

std::string WideToRaw(const std::wstring& str)
{
    std::string result;
    result.reserve(str.size() * 2);
    for (size_t i = 0; i < str.size(); ++i)
    {
        result.push_back((char)((str[i] >> 8) & 0xFF));
        result.push_back((char)(str[i] & 0xFF));
    }
    return result;
}

std::wstring CharToWide(const std::string& str)
{
    std::wstring result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i)
    {
        result.push_back((wchar_t)str[i]);
    }
    return result;
}

std::string WideToChar(const std::wstring& str)
{
    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i)
    {
        result.push_back((char)str[i]);
    }
    return result;
}