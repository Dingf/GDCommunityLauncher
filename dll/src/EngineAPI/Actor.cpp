#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

std::string GetActorDescriptionTag(void* actor)
{
    typedef const char* (__thiscall* GetActorDescriptionTagProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return {};

    GetActorDescriptionTagProto callback = (GetActorDescriptionTagProto)GetProcAddress(engineDLL, EAPI_NAME_GET_ACTOR_DESCRIPTION_TAG);
    if ((!callback) || (!actor))
        return {};

    const char* result = callback(actor);

    if (result)
        return std::string(result);

    return {};
}

}