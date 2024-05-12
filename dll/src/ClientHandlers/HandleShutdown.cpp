#include "ClientHandler.h"
#include "EventManager.h"

void HandleGameInitialize(void* _this)
{
    typedef void (__thiscall* GameInitializeProto)(void*);

    GameInitializeProto callback = (GameInitializeProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GAME_ENGINE_INITIALIZE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_INITIALIZE);
        callback(_this);
    }
}

void HandleGameShutdown(void* _this)
{
    typedef void (__thiscall* GameShutdownProto)(void*);

    GameShutdownProto callback = (GameShutdownProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GAME_ENGINE_SHUTDOWN);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_SHUTDOWN);
        callback(_this);
    }
}