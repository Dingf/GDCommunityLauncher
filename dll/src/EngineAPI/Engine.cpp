#include <Windows.h>
#include "EngineAPI/Engine.h"

namespace EngineAPI
{

void** GetEngineHandle()
{
    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    return (void**)GetProcAddress(engineDLL, EAPI_NAME_ENGINE);
}

}