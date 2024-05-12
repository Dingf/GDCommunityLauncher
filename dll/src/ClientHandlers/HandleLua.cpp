#include "ClientHandler.h"
#include "Version.h"

void RegisterLuaGetVersion(void* _this)
{
    std::string code = "function GDCL_GetVersion() return \"" + std::string(GDCL_VERSION) + "\" end";
    EngineAPI::RunLuaCode(code.c_str());
}

void RegisterLuaGetBranch(void* _this)
{
    Client& client = Client::GetInstance();
    std::string code = "function GDCL_GetBranch() return " + std::to_string(client.GetBranch()) + " end";
    EngineAPI::RunLuaCode(code.c_str());
}

void HandleLuaInitialize(void* _this, bool unk1, bool unk2)
{
    typedef void (__thiscall* LuaInitializeProto)(void*, bool, bool);

    LuaInitializeProto callback = (LuaInitializeProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_LUA_INITIALIZE);
    if (callback)
    { 
        callback(_this, unk1, unk2);

        EngineAPI::SetLuaManager(_this);
        RegisterLuaGetVersion(_this);
        RegisterLuaGetBranch(_this);
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