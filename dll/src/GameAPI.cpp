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