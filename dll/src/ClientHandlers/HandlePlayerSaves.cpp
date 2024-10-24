#include "ClientHandler.h"
#include "EventManager.h"

void HandleSaveNewFormatData(void* _this, void* writer)
{
    typedef void (__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_CHARACTER_PRE_SAVE, _this);
        callback(_this, writer);
        EventManager::Publish(GDCL_EVENT_CHARACTER_POST_SAVE, _this);
    }
}

// The hook for this function is currently disabled
// For some reason, this hook causes the bug where the main menu characters sometimes don't show up
void HandleLoadNewFormatData(void* _this, void* reader)
{
    typedef void (__thiscall* LoadNewFormatDataProto)(void*, void*);

    LoadNewFormatDataProto callback = (LoadNewFormatDataProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_LOAD_NEW_FORMAT_DATA);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_CHARACTER_PRE_LOAD, _this);
        callback(_this, reader);
        EventManager::Publish(GDCL_EVENT_CHARACTER_POST_LOAD, _this);
    }
}