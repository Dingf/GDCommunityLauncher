#include "GameAPI.h"

namespace GameAPI
{

PULONG_PTR GetGameEngineHandle()
{
    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    return (PULONG_PTR)GetProcAddress(gameDLL, GAPI_NAME_GAME_ENGINE);
}

PULONG_PTR GetMainPlayer()
{
    typedef PULONG_PTR(__thiscall* GetMainPlayerProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    GetMainPlayerProto callback = (GetMainPlayerProto)GetProcAddress(gameDLL, GAPI_NAME_GET_MAIN_PLAYER);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    return callback((LPVOID)*gameEngine);
}

const wchar_t* GetPlayerName(PULONG_PTR player)
{
    typedef PULONG_PTR(__thiscall* GetPlayerNameProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!player))
        return nullptr;

    GetPlayerNameProto callback = (GetPlayerNameProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_NAME);
    if (!callback)
        return nullptr;

    return (const wchar_t*)callback((LPVOID)player);
}

Difficulty GetPlayerMaxDifficulty(PULONG_PTR player)
{
    typedef Difficulty(__thiscall* GetPlayerMaxDifficultyProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!player))
        return GAME_DIFFICULTY_NORMAL;

    GetPlayerMaxDifficultyProto callback = (GetPlayerMaxDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY);
    if (!callback)
        return GAME_DIFFICULTY_NORMAL;

    return callback((LPVOID)player);
}

std::string GetBaseFolder()
{
    typedef std::string(__thiscall* GetBaseFolderProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return {};

    GetBaseFolderProto callback = (GetBaseFolderProto)GetProcAddress(gameDLL, GAPI_NAME_GET_BASE_FOLDER);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return {};

    return callback((LPVOID)*gameEngine);
}

Difficulty GetGameDifficulty()
{
    typedef Difficulty(__thiscall* GetGameDifficultyProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return {};

    GetGameDifficultyProto callback = (GetGameDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_GAME_DIFFICULTY);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback((LPVOID)*gameEngine);
}

bool HasToken(PULONG_PTR player, std::string token)
{
    typedef bool(__thiscall* HasTokenProto)(LPVOID, std::string);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!player))
        return nullptr;

    HasTokenProto callback = (HasTokenProto)GetProcAddress(gameDLL, GAPI_NAME_HAS_TOKEN);
    if (!callback)
        return nullptr;

    return callback((LPVOID)player, token);
}

bool IsPlayerHardcore(PULONG_PTR player)
{
    typedef bool(__thiscall* IsPlayerHardcoreProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!player))
        return nullptr;

    IsPlayerHardcoreProto callback = (IsPlayerHardcoreProto)GetProcAddress(gameDLL, GAPI_NAME_IS_HARDCORE);
    if (!callback)
        return nullptr;

    return callback((LPVOID)player);
}

bool IsCloudStorageEnabled()
{
    typedef bool(__thiscall* IsCloudStorageEnabledProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    IsCloudStorageEnabledProto callback = (IsCloudStorageEnabledProto)GetProcAddress(gameDLL, GAPI_NAME_IS_CLOUD_STORAGE);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback((LPVOID)*gameEngine);
}

bool IsGameLoading()
{
    typedef bool(__thiscall* IsGameLoadingProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    IsGameLoadingProto callback = (IsGameLoadingProto)GetProcAddress(gameDLL, GAPI_NAME_IS_GAME_LOADING);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    return callback((LPVOID)*gameEngine);
}

}