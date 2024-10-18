#include <filesystem>
#include <thread>
#include <future>
#include <mutex>
#include "ClientHandler.h"
#include "EventManager.h"
#include "ServerSync.h"

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

        // Only load the transfer stash if it's not synced
        // If it is synced, then don't load the stash as the in-game version should still be accurate
        // This prevents some potential abuse cases with modifying the stash while offline
        if (!ServerSync::IsStashSynced())
            callback(_this);

        EventManager::Publish(GDCL_EVENT_TRANSFER_POST_LOAD);
    }
}

void HandleCaravanInteract(void* _this, uint32_t caravanID, bool unk2, bool unk3)
{
    typedef void (__thiscall* OnCaravanInteractProto)(void*, uint32_t, bool, bool);

    Client& client = Client::GetInstance();
    if (((client.IsInActiveSeason()) && (!client.IsPlayingSeason())) || (ServerSync::IsStashLocked()))
        return;

    // If the stash was not synced, try to sync again and don't let the user open the stash to prevent access to any cheated items
    if (!ServerSync::IsStashSynced())
    {
        GameAPI::LoadTransferStash();
        return;
    }

    OnCaravanInteractProto callback = (OnCaravanInteractProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT);
    if (callback)
    {
        callback(_this, caravanID, unk2, unk3);
    }
}