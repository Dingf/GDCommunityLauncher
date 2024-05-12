#include <array>
#include "StringConvert.h"

std::array<int8_t, 128> _base64Values =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,    // 0x00 - 0x0F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,    // 0x10 - 0x1F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,    // 0x20 - 0x2F
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 64, -1, -1,    // 0x30 - 0x3F
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,    // 0x40 - 0x4F
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,    // 0x50 - 0x5F
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,    // 0x60 - 0x6F
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,    // 0x70 - 0x7F
};

std::array<char, 64> _base64Chars = 
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

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

std::string BinaryToBase64(const std::vector<uint8_t>& str)
{
    std::string result;

    uint32_t size = str.size();
    for (uint32_t i = 0; i < size; i += 3)
    {
        int8_t v1 = str[i];
        result.push_back(_base64Chars[(v1 & 0xFC) >> 2]);
        if ((i + 1) < size)
        {
            int8_t v2 = str[i+1];
            result.push_back(_base64Chars[((v1 & 0x03) << 4) | ((v2 & 0xF0) >> 4)]);

            if ((i + 2) < size)
            {
                int8_t v3 = str[i+2];
                result.push_back(_base64Chars[((v2 & 0x0F) << 2) | ((v3 & 0xC0) >> 6)]);
                result.push_back(_base64Chars[(v3 & 0x3F)]);
            }
            else
            {
                result.push_back(_base64Chars[((v2 & 0x0F) << 2)]);
                result.push_back('=');
            }
        }
        else
        {
            result.push_back(_base64Chars[((v1 & 0x03) << 4)]);
            result.push_back('=');
            result.push_back('=');
        }
    }

    return result;
}

std::vector<uint8_t> Base64ToBinary(const std::string& str)
{
    std::vector<uint8_t> result;

    uint32_t size = str.size();
    for (uint32_t i = 0; i < size; i += 4)
    {
        int8_t v1 = _base64Values[str[i]];
        int8_t v2 = _base64Values[str[i+1]];
        int8_t v3 = _base64Values[str[i+2]];
        int8_t v4 = _base64Values[str[i+3]];

        if ((v1 != -1) && (v2 != -1) && (v3 != -1) && (v4 != -1))
        {
            result.emplace_back(((v1 & 0x3F) << 2) | ((v2 & 0x30) >> 4));
            if (v3 != 64)
                result.emplace_back(((v2 & 0x0F) << 4) | ((v3 & 0x3C) >> 2));
            if (v4 != 64)
                result.emplace_back(((v3 & 0x03) << 6) | ((v4 & 0x3F) >> 0));
        }
    }

    return result;
}