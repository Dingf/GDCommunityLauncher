#include <Windows.h>
#include "EngineAPI/GraphicsTexture.h"

namespace EngineAPI
{

uint32_t GetTextureWidth(void* texture)
{
    typedef uint32_t (__thiscall* GetTextureWidthProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return 0;

    GetTextureWidthProto callback = (GetTextureWidthProto)GetProcAddress(engineDLL, EAPI_NAME_GET_TEXTURE_WIDTH);
    if (!callback)
        return 0;

    return callback(texture);
}

uint32_t GetTextureHeight(void* texture)
{
    typedef uint32_t (__thiscall* GetTextureHeightProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return 0;

    GetTextureHeightProto callback = (GetTextureHeightProto)GetProcAddress(engineDLL, EAPI_NAME_GET_TEXTURE_HEIGHT);
    if (!callback)
        return 0;

    return callback(texture);
}

}