#include <Windows.h>
#include "EngineAPI/Platform.h"

namespace EngineAPI
{

Platform GetPlatform()
{
    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return PLATFORM_UNKNOWN;

    LPVOID steam = GetProcAddress(engineDLL, EAPI_NAME_GET_STEAMWORKS);
    if (steam != nullptr)
        return PLATFORM_STEAM;

    LPVOID gog = GetProcAddress(engineDLL, EAPI_NAME_GET_GOG);
    if (gog != nullptr)
        return PLATFORM_GOG;

    return PLATFORM_UNKNOWN;
}

}