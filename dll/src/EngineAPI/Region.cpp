#include <Windows.h>
#include "EngineAPI/Engine.h"
#include "EngineAPI/Region.h"

namespace EngineAPI
{
    
void* GetEntityRegion(void* entity)
{
    typedef void* (__thiscall* GetEntityRegionProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetEntityRegionProto callback = (GetEntityRegionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_ENTITY_REGION);
    if (!callback)
        return nullptr;

    return callback(entity);
}

void* GetRegionID(void* region)
{
    typedef void* (__thiscall* GetRegionIDProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetRegionIDProto callback = (GetRegionIDProto)GetProcAddress(engineDLL, EAPI_NAME_GET_REGION_ID);
    if (!callback)
        return nullptr;

    return callback(region);
}

std::string GetRegionName(void* region)
{
    typedef std::string* (__thiscall* GetRegionNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return {};

    GetRegionNameProto callback = (GetRegionNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_REGION_NAME);
    if (!callback)
        return {};

    return *callback(region);
}

std::string GetRegionNameTag()
{
    typedef std::string* (__thiscall* GetRegionNameTagProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return {};

    GetRegionNameTagProto callback = (GetRegionNameTagProto)GetProcAddress(engineDLL, EAPI_NAME_GET_REGION_NAME_TAG);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return {};

    return *callback(*engine);
}

}