#ifndef INC_GDCL_DLL_GAME_API_H
#define INC_GDCL_DLL_GAME_API_H

#include <string>
#include <Windows.h>
#include "GameAPI/Difficulty.h"

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PEAVGameEngine@1@EA";
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QEBAPEAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QEBAPEBGXZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QEBA?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_GET_BASE_FOLDER[] = "?GetBaseFolder@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QEBA?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_SET_MAIN_PLAYER[] = "?SetMainPlayer@PlayerManagerClient@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_ON_CARAVAN_INTERACT[] = "?OnPlayerInteract@NpcCaravan@GAME@@UEAAXI_N0@Z";
constexpr char GAPI_NAME_HAS_TOKEN[] = "?HasToken@Player@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_IS_HARDCORE[] = "?IsHardcore@Player@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_IS_CLOUD_STORAGE[] = "?GetCloudStorage@GameEngine@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QEBAXAEAVCheckedWriter@2@@Z";
//constexpr char GAPI_NAME_SAVE_QUEST_STATES_TO_FILE[] = "?SaveQuestStatesToFile@Player@GAME@@QEAAXPEBD@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_LOAD_TRANSFER_STASH[] = "?LoadPlayerTransfer@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_BESTOW_TOKEN[] = "?BestowToken@Player@GAME@@QEAAXAEBUTriggerToken@2@@Z";
constexpr char GAPI_NAME_UNLOAD_WORLD[] = "?UnloadWorld@GameEngine@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_UI_NOTIFY[] = "?QuestCommandUiNotify@GameEngine@GAME@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#else
constexpr char GAPI_NAME_GAME_ENGINE[] = "?gGameEngine@GAME@@3PAVGameEngine@1@A";
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QBEPAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QBEPBGXZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QBE?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_GET_BASE_FOLDER[] = "?GetBaseFolder@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QBE?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_SET_MAIN_PLAYER[] = "?SetMainPlayer@PlayerManagerClient@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_ON_CARAVAN_INTERACT[] = "?OnPlayerInteract@NpcCaravan@GAME@@UAEXI_N0@Z";
constexpr char GAPI_NAME_HAS_TOKEN[] = "?HasToken@Player@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_IS_HARDCORE[] = "?IsHardcore@Player@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_IS_CLOUD_STORAGE[] = "?GetCloudStorage@GameEngine@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_IS_GAME_LOADING[] = "?IsGameLoading@GameEngine@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_SAVE_NEW_FORMAT_DATA[] = "?SaveNewFormatData@Player@GAME@@QBEXAAVCheckedWriter@2@@Z";
//constexpr char GAPI_NAME_SAVE_QUEST_STATES_TO_FILE[] = "?SaveQuestStatesToFile@Player@GAME@@QAEXPBD@Z";
constexpr char GAPI_NAME_SAVE_TRANSFER_STASH[] = "?SaveTransferStash@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_LOAD_TRANSFER_STASH[] = "?LoadPlayerTransfer@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_BESTOW_TOKEN[] = "?BestowToken@Player@GAME@@QAEXABUTriggerToken@2@@Z";
constexpr char GAPI_NAME_UNLOAD_WORLD[] = "?UnloadWorld@GameEngine@GAME@@QAEXXZ";
constexpr char GAPI_NAME_UI_NOTIFY[] = "?QuestCommandUiNotify@GameEngine@GAME@@QAEXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#endif

PULONG_PTR GetGameEngineHandle();

PULONG_PTR GetMainPlayer();

const wchar_t* GetPlayerName(PULONG_PTR player);

Difficulty GetPlayerMaxDifficulty(PULONG_PTR player);

std::string GetBaseFolder();

Difficulty GetGameDifficulty();

bool HasToken(PULONG_PTR player, std::string token);

bool IsPlayerHardcore(PULONG_PTR player);

bool IsCloudStorageEnabled();

bool IsGameLoading();

void DisplayUINotification(const std::string& tag);

}


#endif//INC_GDCL_DLL_GAME_API_H