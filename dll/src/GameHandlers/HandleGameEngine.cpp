#include "GameHandler.h"
#include "EventManager.h"
#include "ContextManager.h"

void HandleGameInitialize(void* _this)
{
    typedef void (__thiscall* GameInitializeProto)(void*);

    GameInitializeProto callback = (GameInitializeProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GAME_ENGINE_INITIALIZE);
    if (callback)
    {
        ContextManager::Run();
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
        EventManager::Publish(GDCL_EVENT_PRE_SHUTDOWN);
        callback(_this);
        EventManager::Publish(GDCL_EVENT_POST_SHUTDOWN);
        ContextManager::Stop();
    }
}

void HandleGameUpdate(void* _this, int32_t unk1)
{
    typedef void (__thiscall* GameUpdateProto)(void*, int32_t);

    GameUpdateProto callback = (GameUpdateProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GAME_ENGINE_UPDATE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_UPDATE);
        callback(_this, unk1);
    }
}

void HandleGameLoading(void* _this, bool state)
{
    typedef void (__thiscall* SetGameLoadingProto)(void*, bool);

    SetGameLoadingProto callback = (SetGameLoadingProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SET_GAME_LOADING);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_LOADING, state);
        callback(_this, state);
    }
}

void HandleSetCloudStorage(void* _this, bool state)
{
    typedef void (__thiscall* SetCloudStorageProto)(void*, bool);

    SetCloudStorageProto callback = (SetCloudStorageProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SET_CLOUD_STORAGE);
    if (callback)
    {
        // Always return false to disable cloud storage while playing in the season
        callback(_this, false);
    }
}

bool HandleGetCloudStorage(void* _this)
{
    // Always return false to disable cloud storage while playing in the season
    return false;
}