#ifndef INC_GDCL_DLL_ENGINE_API_PLATFORM_H
#define INC_GDCL_DLL_ENGINE_API_PLATFORM_H

#include <stdint.h>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_STEAMWORKS[] = "?Get@Steamworks@GAME@@SAPEAV12@XZ";
constexpr char EAPI_NAME_GET_GOG[] = "?Get@Gog@GAME@@SAPEAV12@XZ";
#else
constexpr char EAPI_NAME_GET_STEAMWORKS[] = "?Get@Steamworks@GAME@@SAPAV12@XZ";
constexpr char EAPI_NAME_GET_GOG[] = "?Get@Gog@GAME@@SAPAV12@XZ";
#endif

enum Platform : int32_t
{
    PLATFORM_UNKNOWN = -1,
    PLATFORM_STEAM = 0,
    PLATFORM_GOG = 1,
};

Platform GetPlatform();

}

#endif//INC_GDCL_DLL_ENGINE_API_PLATFORM_H