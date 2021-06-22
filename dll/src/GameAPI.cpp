#include "GameAPI.h"

PULONG_PTR GameAPI::GetGameEngineHandle()
{
    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    return (PULONG_PTR)GetProcAddress(gameDLL, GAPI_NAME_GAME_ENGINE);
}

PULONG_PTR GameAPI::GetMainPlayer()
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

const wchar_t* GameAPI::GetPlayerName(PULONG_PTR player)
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