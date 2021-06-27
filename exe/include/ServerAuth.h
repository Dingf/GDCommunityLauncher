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

    void ValidateCredentials(const std::string& username, const std::string& password, ServerAuthCallback callback);
}

#endif//INC_GDCL_EXE_SERVER_AUTH_H