#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

const std::vector<TriggerToken>& GetPlayerTokens(void* player, Difficulty difficulty)
{
    static std::vector<TriggerToken> empty;
    typedef const std::vector<GameAPI::TriggerToken>& (__thiscall* GetPlayerTokensProto)(void*, GameAPI::Difficulty);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return empty;

    GetPlayerTokensProto callback = (GetPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_TOKENS);
    if (!callback)
        return empty;

    return callback(player, difficulty);
}

const std::vector<TriggerToken>& GetSurvivalTokens(void* player)
{
    static std::vector<TriggerToken> empty;
    typedef const std::vector<TriggerToken>& (__thiscall* GetPlayerTokensProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!player))
        return empty;

    GetPlayerTokensProto callback = (GetPlayerTokensProto)GetProcAddress(gameDLL, GAPI_NAME_GET_SURVIVAL_TOKENS);
    if (!callback)
        return empty;

    return callback(player);
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

}