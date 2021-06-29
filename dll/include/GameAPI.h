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
constexpr char GAPI_NAME_IS_HARDCORE[] = "?IsHardcore@Player@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QEBAXAEAVCheckedWriter@2@@Z";
//constexpr char GAPI_NAME_SAVE_QUEST_STATES_TO_FILE[] = "?SaveQuestStatesToFile@Player@GAME@@QEAAXPEBD@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QEAAXXZ";
#else
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PAVGameEngine@1@A";
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QBEPAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QBEPBGXZ";
constexpr char GAPI_NAME_IS_HARDCORE[] = "?IsHardcore@Player@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QBEXAAVCheckedWriter@2@@Z";
//constexpr char GAPI_NAME_SAVE_QUEST_STATES_TO_FILE[] = "?SaveQuestStatesToFile@Player@GAME@@QAEXPBD@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QAEXXZ";
#endif

PULONG_PTR GetGameEngineHandle();

PULONG_PTR GetMainPlayer();

const wchar_t* GetPlayerName(PULONG_PTR player);

bool IsPlayerHardcore(PULONG_PTR player);

bool IsGameLoading();

}


#endif//INC_GDCL_DLL_GAME_API_H