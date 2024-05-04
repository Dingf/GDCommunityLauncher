#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

void* _luaManager = nullptr;

void SetLuaManager(void* manager)
{
    _luaManager = manager;
}

void* GetLuaManager()
{
    return _luaManager;
}

bool RunLuaCode(const char* code)
{
    typedef void (__thiscall* LuaRunCodeProto)(void*, const char*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    LuaRunCodeProto callback = (LuaRunCodeProto)GetProcAddress(engineDLL, EngineAPI::EAPI_NAME_LUA_RUN_CODE);
    void* lua = GetLuaManager();

    if ((!callback) || (!lua))
        return false;

    callback(lua, code);
    return true;
}

bool LoadLuaFile(const std::string& filename)
{
    typedef void (__thiscall* LoadLuaFileProto)(void*, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    LoadLuaFileProto callback = (LoadLuaFileProto)GetProcAddress(engineDLL, EngineAPI::EAPI_NAME_LUA_LOAD_FILE);
    void* lua = GetLuaManager();

    if ((!callback) || (!lua))
        return false;

    callback(lua, filename);
    return true;
}

}