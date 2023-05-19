#ifndef INC_GDCL_DLL_GAME_API_GAME_ENGINE_H
#define INC_GDCL_DLL_GAME_API_GAME_ENGINE_H

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PEAVGameEngine@1@EA";
constexpr char GAPI_NAME_GAME_ENGINE_SHUTDOWN[] = "?Shutdown@GameEngine@GAME@@QEAAXXZ";
#else
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PAVGameEngine@1@A";
constexpr char GAPI_NAME_GAME_ENGINE_SHUTDOWN[] = "?Shutdown@GameEngine@GAME@@QAEXXZ";
#endif

void** GetGameEngineHandle();

}

#endif//INC_GDCL_DLL_GAME_API_GAME_ENGINE_H