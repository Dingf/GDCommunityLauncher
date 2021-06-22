#include "EngineAPI.h"

PULONG_PTR EngineAPI::GetEngineHandle()
{
    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    return (PULONG_PTR)GetProcAddress(engineDLL, EAPI_NAME_ENGINE);
}

PULONG_PTR EngineAPI::GetGameInfo()
{
    typedef PULONG_PTR(__thiscall* GetGameInfoProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetGameInfoProto callback = (GetGameInfoProto)GetProcAddress(engineDLL, EAPI_NAME_GET_GAME_INFO);
    PULONG_PTR engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback((LPVOID)*engine);
}

const char* EngineAPI::GetModName()
{
    typedef PULONG_PTR(__thiscall* GetModNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetModNameProto callback = (GetModNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_MOD_NAME);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    PULONG_PTR result = callback(gameInfo);
    return (const char*)(*result);
}