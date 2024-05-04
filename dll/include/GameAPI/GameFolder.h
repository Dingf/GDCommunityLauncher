#ifndef INC_GDCL_DLL_GAME_API_GAME_FOLDER_H
#define INC_GDCL_DLL_GAME_API_GAME_FOLDER_H

#include <string>
#include <filesystem>

namespace GameAPI
{

#if _WIN64
    constexpr char GAPI_NAME_GET_ROOT_SAVE_PATH[] = "?GetRootSavePath@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_BASE_FOLDER[] = "?GetBaseFolder@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_USER_SAVE_FOLDER[] = "?GetUserSaveDataFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_FULL_SAVE_FOLDER[] = "?GetFullSaveFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_PLAYER_FOLDER_1[] = "?GetPlayerFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV34@_N1@Z";
    constexpr char GAPI_NAME_GET_PLAYER_FOLDER_2[] = "?GetPlayerFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_MAP_FOLDER[] = "?GetMapFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV34@PEAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_DIFFICULTY_FOLDER[] = "?GetDifficultyFolder@GameEngine@GAME@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4GameDifficulty@2@AEBV34@PEAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_SHARED_SAVE_PATH[] = "?GetSharedSavePath@GameEngine@GAME@@QEBAXW4SharedSave@12@AEAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@_N222@Z";
#else
    constexpr char GAPI_NAME_GET_ROOT_SAVE_PATH[] = "?GetRootSavePath@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_BASE_FOLDER[] = "?GetBaseFolder@GameEngine@GAME@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_USER_SAVE_FOLDER[] = "?GetUserSaveDataFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
    constexpr char GAPI_NAME_GET_FULL_SAVE_FOLDER[] = "?GetFullSaveFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_PLAYER_FOLDER_1[] = "?GetPlayerFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV34@_N1@Z";
    constexpr char GAPI_NAME_GET_PLAYER_FOLDER_2[] = "?GetPlayerFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_MAP_FOLDER[] = "?GetMapFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV34@PAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_DIFFICULTY_FOLDER[] = "?GetDifficultyFolder@GameEngine@GAME@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4GameDifficulty@2@ABV34@PAVPlayer@2@@Z";
    constexpr char GAPI_NAME_GET_SHARED_SAVE_PATH[] = "?GetSharedSavePath@GameEngine@GAME@@QBEXW4SharedSave@12@AAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@_N222@Z";
#endif

enum SharedSaveType
{
    SHARED_SAVE_TRANSFER = 0,
    SHARED_SAVE_FORMULAS = 1,
    SHARED_SAVE_PLAYMENU = 2,
    SHARED_SAVE_LEADERBOARD = 3,
    SHARED_SAVE_TRANSMUTES = 4,
};

void SetRootPrefix(const std::string& prefix);
const std::string& GetRootPrefix();
std::filesystem::path GetBaseFolder();
std::filesystem::path GetUserSaveFolder();
std::filesystem::path GetPlayerFolder(const std::wstring& playerName);
std::filesystem::path GetPlayerFolder(void* player);
std::filesystem::path GetPlayerSaveFile(const std::wstring& playerName);
std::filesystem::path GetPlayerSaveFile(void* player);
std::filesystem::path GetSharedSavePath(GameAPI::SharedSaveType type);
std::filesystem::path GetTransferStashPath();
    
}

#endif//INC_GDCL_DLL_GAME_API_GAME_FOLDER_H