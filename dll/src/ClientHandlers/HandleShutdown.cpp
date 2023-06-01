#include "ClientHandlers.h"
#include "ServerSync.h"

void HandleGameShutdown(void* _this)
{
    typedef void (__thiscall* GameShutdownProto)(void*);

    GameShutdownProto callback = (GameShutdownProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_GAME_ENGINE_SHUTDOWN);
    if (callback)
    {
        ServerSync::UploadCharacterData(false);
        ServerSync::WaitBackgroundComplete();
        callback(_this);
    }
}