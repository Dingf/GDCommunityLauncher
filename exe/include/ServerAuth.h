#ifndef INC_GDCL_EXE_SERVER_AUTH_H
#define INC_GDCL_EXE_SERVER_AUTH_H

#include <string>

enum ServerAuthResult
{
    SERVER_AUTH_TIMEOUT = -1,
    SERVER_AUTH_FAIL = 0,
    SERVER_AUTH_OK = 1,
};

typedef void(*ServerAuthCallback)(ServerAuthResult);

namespace ServerAuth
{
    void ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password, ServerAuthCallback callback);

    ServerAuthResult ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password);
}

#endif//INC_GDCL_EXE_SERVER_AUTH_H