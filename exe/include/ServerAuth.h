#ifndef INC_GDCL_EXE_SERVER_AUTH_H
#define INC_GDCL_EXE_SERVER_AUTH_H

#include <string>

enum ServerAuthResult
{
    SERVER_AUTH_OK = 0,
    SERVER_AUTH_INVALID_LOGIN = 1,
    SERVER_AUTH_TIMEOUT = 2,
    SERVER_AUTH_NO_ACTIVE_SEASON = 3,
    SERVER_AUTH_OTHER_ERROR = 4,
};

typedef void (*ServerAuthCallback)(ServerAuthResult);

ServerAuthResult ServerAuthenticate(ServerAuthCallback callback);

#endif//INC_GDCL_EXE_SERVER_AUTH_H