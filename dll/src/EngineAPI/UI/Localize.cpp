#include <Windows.h>
#include "EngineAPI/UI/Localize.h"

namespace EngineAPI::UI
{

void* GetLocalizationManager()
{
    typedef void* (__thiscall* GetLocalizationManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetLocalizationManagerProto callback = (GetLocalizationManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_LOCALIZATION_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

}