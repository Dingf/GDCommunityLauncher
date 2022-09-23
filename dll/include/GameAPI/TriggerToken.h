#ifndef INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H
#define INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H

#include <stdint.h>
#include <vector>
#include <Windows.h>
#include "GameAPI/Difficulty.h"

namespace GameAPI
{

struct TriggerToken
{
    const char* GetTokenString() const
    {
        if (_bufferLength <= 0x0F)
            return (const char*)(&_token1);
        else
            return (const char*)_token1;
    }

    uint64_t _unk1;             // A pointer to some API function, always constant
    uint64_t _token1;           // If bufferLength <= 0x0F, this and _token2 form a 16-bit buffer that holds the string
    uint64_t _token2;           // Otherwise, _token1 is a pointer to the actual string and _token2 is ignored
    size_t   _tokenLength;      // The length of the token string
    size_t   _bufferLength;     // The length of the buffer used to store the token string
};

const std::vector<TriggerToken>& GetPlayerTokens(PULONG_PTR player, Difficulty difficulty);

}

#endif//INC_GDCL_DLL_GAME_API_TRIGGER_TOKEN_H