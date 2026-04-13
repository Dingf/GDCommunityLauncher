#include "GameHandler.h"
#include "EventManager.h"

void HandleExitPlayingMode(void* _this)
{
    typedef void (__thiscall* ExitPlayingModeProto)(void*);

    ExitPlayingModeProto callback = (ExitPlayingModeProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_EXIT_PLAYING_MODE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_EXIT_PLAYING_MODE);
        callback(_this);
    }
}