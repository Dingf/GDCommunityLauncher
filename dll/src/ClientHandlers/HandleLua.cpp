#include "ClientHandler.h"
#include "Version.h"

void HandleLuaInitialize(void* _this, bool unk1, bool unk2)
{
    typedef void (__thiscall* LuaInitializeProto)(void*, bool, bool);

    LuaInitializeProto callback = (LuaInitializeProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_LUA_INITIALIZE);
    if (callback)
    {
        callback(_this, unk1, unk2);

        std::string code = "function GDCL_GetVersion() return \"" + std::string(GDCL_VERSION) + "\" end";

        EngineAPI::SetLuaManager(_this);
        EngineAPI::RunLuaCode(code.c_str());
    }
}

void HandleLuaShutdown(void* _this)
{
    typedef void (__thiscall* LuaShutdownProto)(void*);

    LuaShutdownProto callback = (LuaShutdownProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_LUA_SHUTDOWN);
    if (callback)
    {
        callback(_this);

        EngineAPI::SetLuaManager(nullptr);
    }
}