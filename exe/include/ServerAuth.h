#ifndef INC_GDCL_EXE_SERVER_AUTH_H
#define INC_GDCL_EXE_SERVER_AUTH_H

#include <string>
#include "ClientBase.h"

enum ServerAuthResult
{
    SERVER_AUTH_CALLBACK = -1,
    SERVER_AUTH_OK = 0,
    SERVER_AUTH_INVALID_LOGIN = 1,
    SERVER_AUTH_TIMEOUT = 2,
};

typedef void (*ServerAuthCallback)(ServerAuthResult, const ClientData&);

ServerAuthResult ServerAuthenticate(ClientData& data, ServerAuthCallback callback);

#endif//INC_GDCL_EXE_SERVER_AUTH_H