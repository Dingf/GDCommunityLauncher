#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

void** GetEngineHandle()
{
    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    return (void**)GetProcAddress(engineDLL, EAPI_NAME_ENGINE);
}


std::string GetVersionString()
{
    typedef const char* (__thiscall* GetVersionStringProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return {};

    GetVersionStringProto callback = (GetVersionStringProto)GetProcAddress(engineDLL, EAPI_NAME_GET_VERSION);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return {};

    const char* result = callback(*engine);

    if (result)
        return std::string(result);

    return {};
}

bool InitializeMod(const std::string& modName)
{
    typedef bool (__thiscall* InitializeModProto)(void*, const std::string&, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    InitializeModProto callback = (InitializeModProto)GetProcAddress(engineDLL, EAPI_NAME_INITIALIZE_MOD);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return false;

    // Not sure what the second option here is for yet; so far it has only been empty
    std::string unk1;
    return callback(*engine, modName, unk1);
}

bool LoadDatabase(const std::string& database)
{
    typedef bool (__thiscall* LoadDatabaseProto)(void*, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    LoadDatabaseProto callback = (LoadDatabaseProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_DATABASE);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return false;

    return callback(*engine, database);
}

bool IsExpansion1Loaded()
{
    typedef bool (__thiscall* IsExpansion1LoadedProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    IsExpansion1LoadedProto callback = (IsExpansion1LoadedProto)GetProcAddress(engineDLL, EAPI_NAME_IS_EXPANSION1_LOADED);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return false;

    return callback(*engine);
}

bool IsExpansion2Loaded()
{
    typedef bool (__thiscall* IsExpansion2LoadedProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    IsExpansion2LoadedProto callback = (IsExpansion2LoadedProto)GetProcAddress(engineDLL, EAPI_NAME_IS_EXPANSION2_LOADED);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return false;

    return callback(*engine);
}

bool IsExpansion3Loaded()
{
    typedef bool (__thiscall* IsExpansion3LoadedProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    IsExpansion3LoadedProto callback = (IsExpansion3LoadedProto)GetProcAddress(engineDLL, EAPI_NAME_IS_EXPANSION3_LOADED);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return false;

    return callback(*engine);
}

}