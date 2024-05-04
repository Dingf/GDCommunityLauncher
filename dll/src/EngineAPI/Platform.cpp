#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

Platform GetPlatform()
{
    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return PLATFORM_UNKNOWN;

    if (LPVOID steam = GetProcAddress(engineDLL, EAPI_NAME_GET_STEAMWORKS))
        return PLATFORM_STEAM;

    if (LPVOID gog = GetProcAddress(engineDLL, EAPI_NAME_GET_GOG))
        return PLATFORM_GOG;

    return PLATFORM_UNKNOWN;
}

}