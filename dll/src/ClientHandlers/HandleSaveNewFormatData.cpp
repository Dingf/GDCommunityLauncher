#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "UpdateThread.h"
#include "Character.h"
#include "Quest.h"
#include "URI.h"
#include "Log.h"

void HandleSaveNewFormatData(void* _this, void* writer)
{
    typedef void(__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        callback(_this, writer);

        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();
        if ((modName) && (mainPlayer) && (!EngineAPI::IsMultiplayer()) && (client.IsParticipatingInSeason()))
        {
            client.UpdateCharacterData(5000, true);
        }
    }
}