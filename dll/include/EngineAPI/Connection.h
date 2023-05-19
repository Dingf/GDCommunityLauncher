#ifndef INC_GDCL_DLL_ENGINE_API_CONNECTION_H
#define INC_GDCL_DLL_ENGINE_API_CONNECTION_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_CREATE_SERVER_CONNECTION[] = "?CreateNewConnection@ServerConnectionManager@GAME@@AEAAHPEAVNetworkConnection@2@PEBVNetworkAddress@2@1H@Z";
constexpr char EAPI_NAME_ADD_NETWORK_SERVER[] = "?AddServer@NetworkServerBrowser@GAME@@QEAAXPEAUServerInfo@2@W4NetworkInfrastructureType@2@@Z";
#else
constexpr char EAPI_NAME_CREATE_SERVER_CONNECTION[] = "?CreateNewConnection@ServerConnectionManager@GAME@@AAEHPAVNetworkConnection@2@PBVNetworkAddress@2@1H@Z";
constexpr char EAPI_NAME_ADD_NETWORK_SERVER[] = "?AddServer@NetworkServerBrowser@GAME@@QAEXPAUServerInfo@2@W4NetworkInfrastructureType@2@@Z";
#endif

}


#endif//INC_GDCL_DLL_ENGINE_API_CONNECTION_H