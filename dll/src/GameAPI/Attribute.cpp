#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

float GetTotalCharAttribute(void* character, CharAttributeType type)
{
    typedef float (__thiscall* GetTotalCharAttributeProto)(void*, CharAttributeType);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0.0f;

    GetTotalCharAttributeProto callback = (GetTotalCharAttributeProto)GetProcAddress(gameDLL, GAPI_NAME_GET_TOTAL_CHAR_ATTRIBUTE);
    if ((!callback) || (!character))
        return 0.0f;

    return callback(character, type);
}

float GetTotalCharModifier(void* character, CharAttributeType type)
{
    typedef float (__thiscall* GetTotalCharAttributeProto)(void*, CharAttributeType);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0.0f;

    GetTotalCharAttributeProto callback = (GetTotalCharAttributeProto)GetProcAddress(gameDLL, GAPI_NAME_GET_TOTAL_CHAR_MODIFIER);
    if ((!callback) || (!character))
        return 0.0f;

    return callback(character, type);
}

double GetCurrentLife(void* character)
{
    typedef double (__thiscall* GetCurrentLifeProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0.0;

    GetCurrentLifeProto callback = (GetCurrentLifeProto)GetProcAddress(gameDLL, GAPI_NAME_GET_CURRENT_LIFE);
    if ((!callback) || (!character))
        return 0.0;

    return callback(character);
}

}
