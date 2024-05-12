#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void* GetSkillManager(void* character)
{
    typedef void* (__thiscall* GetSkillManagerProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetSkillManagerProto callback = (GetSkillManagerProto)GetProcAddress(gameDLL, GAPI_NAME_GET_SKILL_MANAGER);
    if ((!callback) || (!character))
        return nullptr;

    return callback(character);
}

std::string GetSkillNameTag(void* skill)
{
    typedef const std::string& (__thiscall* GetSkillNameTagProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    GetSkillNameTagProto callback = (GetSkillNameTagProto)GetProcAddress(gameDLL, GAPI_NAME_GET_SKILL_NAME_TAG);
    if ((!callback) || (!skill))
        return {};

    return callback(skill);
}

}