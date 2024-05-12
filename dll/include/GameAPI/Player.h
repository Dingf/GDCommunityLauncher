#ifndef INC_GDCL_DLL_GAME_API_PLAYER_H
#define INC_GDCL_DLL_GAME_API_PLAYER_H

#include <stdint.h>
#include <string>
#include "GameAPI/Difficulty.h"

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QEBAPEAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_CONTROLLER_ID[] = "?GetControllerId@Character@GAME@@QEBA?BIXZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QEBAPEBGXZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME_IN_CHAR[] = "?GetPlayerNameInChar@Player@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char GAPI_NAME_GET_PLAYER_PARTY_ID[] = "?GetPartyId@Player@GAME@@QEBAHXZ";
constexpr char GAPI_NAME_GET_PLAYER_HAS_BEEN_IN_GAME[] = "?GetHasBeenInGame@Player@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QEBA?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_SET_PLAYER_MAX_DIFFICULTY[] = "?SetGreatestDifficultyCompleted@Player@GAME@@QEAAXW4GameDifficulty@2@@Z";
constexpr char GAPI_NAME_HAS_TOKEN[] = "?HasToken@Player@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_BESTOW_TOKEN[] = "?BestowToken@Player@GAME@@QEAAXAEBUTriggerToken@2@@Z";
constexpr char GAPI_NAME_IS_PLAYER_HARDCORE[] = "?IsHardcore@Player@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_IS_PLAYER_IN_MAIN_QUEST[] = "?IsInMainQuest@Player@GAME@@QEBA_NXZ";
constexpr char GAPI_NAME_SET_MAIN_PLAYER[] = "?SetMainPlayer@PlayerManagerClient@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_GIVE_ITEM_TO_PLAYER[] = "?GiveItemToCharacter@Player@GAME@@UEAAXPEAVItem@2@_N1@Z";
constexpr char GAPI_NAME_LOAD_PLAYER_QUEST_STATES[] = "?LoadQuestStatesFromFile@Player@GAME@@QEAAXPEBD@Z";
constexpr char GAPI_NAME_SAVE_PLAYER_QUEST_STATES[] = "?SaveQuestStatesToFile@Player@GAME@@QEAAXPEBD@Z";
constexpr char GAPI_NAME_SET_INVINCIBLE[] = "?SetInvincible@Character@GAME@@QEAAX_N@Z";
constexpr char GAPI_NAME_SET_GOD[] = "?SetGod@Character@GAME@@QEAAX_N@Z";
constexpr char GAPI_NAME_ADD_MONEY[] = "?AddMoney@Character@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_SUBTRACT_MONEY[] = "?SubtractMoney@Character@GAME@@QEAA?BII@Z";
#else
constexpr char GAPI_NAME_GET_MAIN_PLAYER[] = "?GetMainPlayer@GameEngine@GAME@@QBEPAVPlayer@2@XZ";
constexpr char GAPI_NAME_GET_CONTROLLER_ID[] = "?GetControllerId@Character@GAME@@QBE?BIXZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME[] = "?GetPlayerName@Player@GAME@@QBEPBGXZ";
constexpr char GAPI_NAME_GET_PLAYER_NAME_IN_CHAR[] = "?GetPlayerNameInChar@Player@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char GAPI_NAME_GET_PLAYER_PARTY_ID[] = "?GetPartyId@Player@GAME@@QBEHXZ";
constexpr char GAPI_NAME_GET_PLAYER_HAS_BEEN_IN_GAME[] = "?GetHasBeenInGame@Player@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QBE?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_SET_PLAYER_MAX_DIFFICULTY[] = "?SetGreatestDifficultyCompleted@Player@GAME@@QAEXW4GameDifficulty@2@@Z";
constexpr char GAPI_NAME_HAS_TOKEN[] = "?HasToken@Player@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char GAPI_NAME_BESTOW_TOKEN[] = "?BestowToken@Player@GAME@@QAEXABUTriggerToken@2@@Z";
constexpr char GAPI_NAME_IS_PLAYER_HARDCORE[] = "?IsHardcore@Player@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_IS_PLAYER_IN_MAIN_QUEST[] = "?IsInMainQuest@Player@GAME@@QBE_NXZ";
constexpr char GAPI_NAME_SET_MAIN_PLAYER[] = "?SetMainPlayer@PlayerManagerClient@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_GIVE_ITEM_TO_PLAYER[] = "?GiveItemToCharacter@Player@GAME@@UAEXPAVItem@2@_N1@Z";
constexpr char GAPI_NAME_LOAD_PLAYER_QUEST_STATES[] = "?LoadQuestStatesFromFile@Player@GAME@@QAEXPBD@Z";
constexpr char GAPI_NAME_SAVE_PLAYER_QUEST_STATES[] = "?SaveQuestStatesToFile@Player@GAME@@QAEXPBD@Z";
constexpr char GAPI_NAME_SET_INVINCIBLE[] = "?SetInvincible@Character@GAME@@QAEX_N@Z";
constexpr char GAPI_NAME_SET_GOD[] = "?SetGod@Character@GAME@@QAEX_N@";
constexpr char GAPI_NAME_ADD_MONEY[] = "?AddMoney@Character@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_SUBTRACT_MONEY[] = "?SubtractMoney@Character@GAME@@QAE?BII@Z";
#endif

void* GetMainPlayer();
void* GetPlayerController(void* player);
const wchar_t* GetPlayerName(void* player);
//std::string GetPlayerNameInChar(void* player);        // This function crashes the game for some reason? Use GetPlayerName() instead and convert to std::string as needed
int32_t GetPlayerPartyID(void* player);
bool PlayerHasToken(void* player, std::string token);
bool IsPlayerHardcore(void* player);
bool IsPlayerInMainQuest(void* player);
bool HasPlayerBeenInGame(void* player);
Difficulty GetPlayerMaxDifficulty(void* player);
void SetPlayerMaxDifficulty(void* player, Difficulty difficulty);
void GiveItemToPlayer(void* player, void* item, bool unk1, bool unk2);
void LoadQuestStatesFromFile(void* player, const char* filename);
void SaveQuestStatesToFile(void* player, const char* filename);
void AddOrSubtractMoney(void* player, int32_t amount);

}

#endif//INC_GDCL_DLL_GAME_API_PLAYER_H