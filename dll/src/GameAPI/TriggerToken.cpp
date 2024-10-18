#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

// Dummy vector to be returned in the event of failure
std::vector<TriggerToken> dummy;

const std::vector<TriggerToken>& GetPlayerTokens(void* player, Difficulty difficulty)
{
    typedef const std::vector<GameAPI::TriggerToken>& (__thiscall* GetPlayerTokensProto)(void*, GameAPI::Difficulty);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return dummy;

    GetPlayerTokensProto callback = (GetPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_TOKENS);
    if (!callback)
        return dummy;

    return callback(player, difficulty);
}

void BestowTokenNow(void* player, const std::string& token)
{
    typedef void (__thiscall* GetPlayerTokensProto)(void*, const std::string&);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return;

    GetPlayerTokensProto callback = (GetPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_BESTOW_TOKEN_NOW);
    if (!callback)
        return;

    return callback(player, token);
}

void ClearPlayerTokens(void* player)
{
    typedef void (__thiscall* ClearPlayerTokensProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return;

    ClearPlayerTokensProto callback = (ClearPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_CLEAR_PLAYER_TOKENS);
    if (!callback)
        return;

    callback((LPVOID)player);
}

void ClearPlayerTokens(void* player)
{
    typedef void (__thiscall* ClearPlayerTokensProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return;

    ClearPlayerTokensProto callback = (ClearPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_CLEAR_PLAYER_TOKENS);
    if (!callback)
        return;

    callback((LPVOID)player);
}

}