#ifndef INC_GDCL_MD5_H
#define INC_GDCL_MD5_H

#include <string>
#include <sstream>
#include <filesystem>
#include "external/md5.hpp"

inline std::string GenerateFileMD5(const std::filesystem::path& path)
{
    std::stringstream buffer;
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return {};

    buffer << in.rdbuf();

    in.close();

    // Capitalize the MD5 hash to match the server output
    std::string result = websocketpp::md5::md5_hash_hex(buffer.str());
    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        *it = toupper(*it);

    return result;
}

#endif//INC_GDCL_MD5_H