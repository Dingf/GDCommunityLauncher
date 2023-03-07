#ifndef INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H
#define INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H

#include <stdint.h>
#include <string>
#include <vector>
#include <Windows.h>
#include "GameAPI/Difficulty.h"

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GET_PLAYER_TOKENS[] = "?GetTokens@Player@GAME@@QEBAAEBV?$vector@UTriggerToken@GAME@@@mem@@W4GameDifficulty@2@@Z";
#else
constexpr char GAPI_NAME_GET_PLAYER_TOKENS[] = "?GetTokens@Player@GAME@@QBEABV?$vector@UTriggerToken@GAME@@@mem@@W4GameDifficulty@2@@Z";
#endif

struct TriggerToken
{
    operator std::string() const
    {
        return _token;
    }

    uint64_t _unk1;             // A pointer to some API function, always constant
    std::string _token;         // The token string value
};

const std::vector<TriggerToken>& GetPlayerTokens(PULONG_PTR player, Difficulty difficulty);

}

#endif//INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H