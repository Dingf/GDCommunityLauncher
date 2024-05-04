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

    return callback((LPVOID)player, difficulty);
}

}