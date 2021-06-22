#ifndef INC_GDCL_DLL_GAME_API_H
#define INC_GDCL_DLL_GAME_API_H

#include <string>
#include <Windows.h>

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PEAVGameEngine@1@EA";
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QEBAPEAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QEBAPEBGXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QEBAXAEAVCheckedWriter@2@@Z";
#else
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PAVGameEngine@1@A";
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QBEPAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QBEPBGXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QBEXAAVCheckedWriter@2@@Z";
#endif

PULONG_PTR GetGameEngineHandle();

PULONG_PTR GetMainPlayer();

const wchar_t* GetPlayerName(PULONG_PTR player);

}


#endif//INC_GDCL_DLL_GAME_API_H