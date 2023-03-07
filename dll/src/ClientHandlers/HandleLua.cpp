#include "ClientHandlers.h"
#include "Version.h"

void LuaRunCode(void* _this, const char* code)
{
    typedef void(__thiscall* LuaRunCodeProto)(void*, const char*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return;

    LuaRunCodeProto callback = (LuaRunCodeProto)GetProcAddress(engineDLL, EngineAPI::EAPI_NAME_LUA_RUN_CODE);
    if (callback)
    {
        callback(_this, code);
    }
}

void HandleLuaInitialize(void* _this, bool unk1, bool unk2)
{
    typedef void(__thiscall* LuaInitializeProto)(void*, bool, bool);

    LuaInitializeProto callback = (LuaInitializeProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LUA_INITIALIZE);
    if (callback)
    {
        callback(_this, unk1, unk2);

        std::string code = "function GDCL_GetVersion() return \"" + std::string(GDCL_VERSION) + "\" end";
        LuaRunCode(_this, code.c_str());
    }
}