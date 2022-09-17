#ifndef INC_GDCL_DLL_SERVER_AUTH_EXPORT_H
#define INC_GDCL_DLL_SERVER_AUTH_EXPORT_H

#include <string>
#include "ClientBase.h"

enum ServerAuthResult
{
    SERVER_AUTH_CALLBACK = -1,
    SERVER_AUTH_OK = 0,
    SERVER_AUTH_INVALID_LOGIN = 1,
    SERVER_AUTH_TIMEOUT = 2,
    SERVER_AUTH_INVALID_SEASONS = 3,
    SERVER_AUTH_UNKNOWN_ERR = 4,
};

typedef void(*ServerAuthCallback)(ServerAuthResult, const ClientData&);

__declspec(dllexport) std::string GetLauncherVersion(const std::string& hostName);
__declspec(dllexport) ServerAuthResult ServerAuthenticate(ClientData& data, const std::string& password, ServerAuthCallback callback);

#endif//INC_GDCL_DLL_SERVER_AUTH_EXPORT_H