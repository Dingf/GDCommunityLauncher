#include <Windows.h>
#include "EngineAPI.h"
#include "GameAPI.h"

namespace GameAPI
{

void* GetMainPlayer()
{
    typedef void* (__thiscall* GetMainPlayerProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetMainPlayerProto callback = (GetMainPlayerProto)GetProcAddress(gameDLL, GAPI_NAME_GET_MAIN_PLAYER);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    return callback(*gameEngine);
}

void* GetPlayerController(void* player)
{
    if (!player)
        return nullptr;

    typedef uint32_t (__thiscall* GetControllerIDProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetControllerIDProto callback = (GetControllerIDProto)GetProcAddress(gameDLL, GAPI_NAME_GET_CONTROLLER_ID);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    uint32_t controllerID = callback(player);
    return EngineAPI::FindObjectByID(controllerID);
}

const wchar_t* GetPlayerName(void* player)
{
    typedef const wchar_t* (__thiscall* GetPlayerNameProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return nullptr;

    GetPlayerNameProto callback = (GetPlayerNameProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_NAME);
    if (!callback)
        return nullptr;

    return callback((void*)player);
}

std::string GetPlayerNameInChar(void* player)
{
    typedef std::string (__thiscall* GetPlayerNameInCharProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return {};

    GetPlayerNameInCharProto callback = (GetPlayerNameInCharProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_NAME_IN_CHAR);
    if (!callback)
        return {};

    return callback((void*)player);
}

int32_t GetPlayerPartyID(void* player)
{
    typedef int32_t(__thiscall* GetPlayerPartyIDProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return 0;

    GetPlayerPartyIDProto callback = (GetPlayerPartyIDProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_NAME);
    if (!callback)
        return 0;

    return callback((void*)player);
}

bool PlayerHasToken(void* player, std::string token)
{
    typedef bool(__thiscall* HasTokenProto)(void*, std::string);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return nullptr;

    HasTokenProto callback = (HasTokenProto)GetProcAddress(gameDLL, GAPI_NAME_HAS_TOKEN);
    if (!callback)
        return nullptr;

    return callback((void*)player, token);
}

bool IsPlayerHardcore(void* player)
{
    typedef bool(__thiscall* IsPlayerHardcoreProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return nullptr;

    IsPlayerHardcoreProto callback = (IsPlayerHardcoreProto)GetProcAddress(gameDLL, GAPI_NAME_IS_PLAYER_HARDCORE);
    if (!callback)
        return nullptr;

    return callback((void*)player);
}

bool IsPlayerInMainQuest(void* player)
{
    typedef bool(__thiscall* IsPlayerInMainQuestProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return nullptr;

    IsPlayerInMainQuestProto callback = (IsPlayerInMainQuestProto)GetProcAddress(gameDLL, GAPI_NAME_IS_PLAYER_IN_MAIN_QUEST);
    if (!callback)
        return nullptr;

    return callback((void*)player);
}

bool HasPlayerBeenInGame(void* player)
{
    typedef bool(__thiscall* HasPlayerBeenInGameProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return nullptr;

    HasPlayerBeenInGameProto callback = (HasPlayerBeenInGameProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_HAS_BEEN_IN_GAME);
    if (!callback)
        return nullptr;

    return callback((void*)player);
}

Difficulty GetPlayerMaxDifficulty(void* player)
{
    typedef Difficulty(__thiscall* GetPlayerMaxDifficultyProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return GAME_DIFFICULTY_NORMAL;

    GetPlayerMaxDifficultyProto callback = (GetPlayerMaxDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY);
    if (!callback)
        return GAME_DIFFICULTY_NORMAL;

    return callback(player);
}

void SetPlayerMaxDifficulty(void* player, Difficulty difficulty)
{
    typedef void (__thiscall* SetPlayerMaxDifficultyProto)(void*, Difficulty);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return;

    SetPlayerMaxDifficultyProto callback = (SetPlayerMaxDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_SET_PLAYER_MAX_DIFFICULTY);
    if (!callback)
        return;

    callback(player, difficulty);
}

void GiveItemToPlayer(void* player, void* item, bool unk1, bool unk2)
{
    typedef void (__thiscall* GiveItemToCharacterProto)(void*, void*, bool, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    GiveItemToCharacterProto callback = (GiveItemToCharacterProto)GetProcAddress(gameDLL, GAPI_NAME_GIVE_ITEM_TO_PLAYER);
    if ((!callback) || (!player) || (!item))
        return;

    callback(player, item, unk1, unk2);
}

void AddOrSubtractMoney(void* player, int32_t amount)
{
    if (!player)
        return;

    uint32_t* moneyAddress = (uint32_t*)((uintptr_t)player + 0x15E4);
    int64_t moneyAmount = *moneyAddress;
    moneyAmount += amount;
    moneyAmount = std::clamp(moneyAmount, (int64_t)0, (int64_t)0xFFFFFFFF);
    *moneyAddress = (uint32_t)moneyAmount;
}

}