#include <filesystem>
#include <thread>
#include <future>
#include <mutex>
#include "GameHandler.h"
#include "EventManager.h"

void HandleSaveTransferStash(void* _this)
{
    typedef void (__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_TRANSFER_PRE_SAVE);
        callback(_this);
        EventManager::Publish(GDCL_EVENT_TRANSFER_POST_SAVE);
    }
}

void HandleLoadTransferStash(void* _this)
{
    typedef void (__thiscall* LoadPlayerTransferProto)(void*);
    LoadPlayerTransferProto callback = (LoadPlayerTransferProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);

    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_TRANSFER_PRE_LOAD);
        callback(_this);
        EventManager::Publish(GDCL_EVENT_TRANSFER_POST_LOAD);
    }
}

void HandleCaravanInteract(void* _this, uint32_t caravanID, bool unk2, bool unk3)
{
    typedef void (__thiscall* OnCaravanInteractProto)(void*, uint32_t, bool, bool);

    OnCaravanInteractProto callback = (OnCaravanInteractProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT);
    if ((callback) && (EventManager::Poll(GDCL_EVENT_CARAVAN_INTERACT, EngineAPI::GetObjectID(_this))))
    {
        callback(_this, caravanID, unk2, unk3);
    }
}