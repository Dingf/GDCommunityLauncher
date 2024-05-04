#ifndef INC_GDCL_MD5_H
#define INC_GDCL_MD5_H

#include <string>
#include <sstream>
#include <filesystem>
#include "external/md5.hpp"

inline std::string GenerateFileMD5(const std::filesystem::path& path)
{
    std::stringstream stream;
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return {};

    stream << in.rdbuf();

    in.close();

    // Capitalize the MD5 hash to match the server output
    std::string result = websocketpp::md5::md5_hash_hex(stream.str());
    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        *it = toupper(*it);

    return result;
}

inline std::string GenerateBufferMD5(uint8_t* buffer, size_t size)
{
    std::stringstream stream;
    stream.write((const char*)buffer, size);

    // Capitalize the MD5 hash to match the server output
    std::string result = websocketpp::md5::md5_hash_hex(stream.str());
    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        *it = toupper(*it);

    return result;
}

#endif//INC_GDCL_MD5_H