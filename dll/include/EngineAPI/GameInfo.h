#ifndef INC_GDCL_DLL_ENGINE_API_GAME_INFO_H
#define INC_GDCL_DLL_ENGINE_API_GAME_INFO_H

#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QEAAPEAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_PLAYER_LEVEL[] = "?GetPlayerLevel@GameInfo@GAME@@QEBAIXZ";
constexpr char EAPI_NAME_GET_LEVEL_NAME[] = "?GetLevelName@GameInfo@GAME@@QEAAXAEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@@Z";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_HARDCORE[] = "?GetHardcore@GameInfo@GAME@@QEBA_NXZ";
constexpr char EAPI_NAME_GET_IS_MULTIPLAYER[] = "?GetIsMultiPlayer@GameInfo@GAME@@QEBA_NXZ";
#else
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QAEPAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_PLAYER_LEVEL[] = "?GetPlayerLevel@GameInfo@GAME@@QBEIXZ";
constexpr char EAPI_NAME_GET_LEVEL_NAME[] = "?GetLevelName@GameInfo@GAME@@QAEXAAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@@Z";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_HARDCORE[] = "?GetHardcore@GameInfo@GAME@@QBE_NXZ";
constexpr char EAPI_NAME_GET_IS_MULTIPLAYER[] = "?GetIsMultiPlayer@GameInfo@GAME@@QBE_NXZ";
#endif

void* GetGameInfo();
uint32_t GetPlayerLevel();
std::wstring GetLevelName();
std::string GetModName();
bool IsHardcore();
bool IsMultiplayer();
bool IsMainCampaign();
bool IsMainCampaignOrCrucible();

}

#endif//INC_GDCL_DLL_ENGINE_API_GAME_INFO_H