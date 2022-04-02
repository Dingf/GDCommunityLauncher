#include "ClientHandlers.h"
#include "Version.h"

void HandleLuaInitialize(void* _this, bool unk1, bool unk2)
{
    typedef void(__thiscall* LuaInitializeProto)(void*, bool, bool);

    LuaInitializeProto callback = (LuaInitializeProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LUA_INITIALIZE);
    if (callback)
    {
        callback(_this, unk1, unk2);

        typedef void(__thiscall* LuaRunCodeProto)(void*, const char*);
        LuaRunCodeProto luaRunCode = (LuaRunCodeProto)GetProcAddress(GetModuleHandle(TEXT("Engine.dll")), EngineAPI::EAPI_NAME_LUA_RUN_CODE);

        std::string code = "function GDCL_GetVersion() return \"" + std::string(GDCL_VERSION) + "\" end";
        luaRunCode(_this, code.c_str());
    }
}