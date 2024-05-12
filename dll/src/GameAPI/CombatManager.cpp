#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void* GetCombatManager(void* character)
{
    typedef void* (__thiscall* GetCombatManagerProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetCombatManagerProto callback = (GetCombatManagerProto)GetProcAddress(gameDLL, GAPI_NAME_GET_COMBAT_MANAGER);
    if ((!callback) || (!character))
        return nullptr;

    return callback(character);
}

float GetCharacterOA(void* character)
{
    typedef float (__thiscall* GetCharacterOAProto)(void*, float);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0.0f;

    GetCharacterOAProto callback = (GetCharacterOAProto)GetProcAddress(gameDLL, GAPI_NAME_CALCULATE_OFFENSIVE_ABILITY);
    void* manager = GetCombatManager(character);
    if ((!callback) || (!manager))
        return 0.0f;

    return callback(manager, 0.0f);
}

float GetCharacterDA(void* character)
{
    typedef float (__thiscall* GetCharacterDAProto)(void*, float);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0.0f;

    GetCharacterDAProto callback = (GetCharacterDAProto)GetProcAddress(gameDLL, GAPI_NAME_CALCULATE_DEFENSIVE_ABILITY);
    void* manager = GetCombatManager(character);
    if ((!callback) || (!manager))
        return 0.0f;

    return callback(manager, 0.0f);
}

}