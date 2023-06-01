#include <filesystem>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ServerSync.h"

void HandleSaveNewFormatData(void* _this, void* writer)
{
    typedef void (__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsParticipatingInSeason()) && (!EngineAPI::IsMultiplayer()))
        {
            std::wstring playerName = client.GetActiveCharacterName();
            if (client.IsInProductionBranch())
                ServerSync::SnapshotCharacterMetadata(playerName);

            callback(_this, writer);
            ServerSync::UploadCharacterData(true);
        }
        else
        {
            callback(_this, writer);
        }
    }
}

void HandleLoadNewFormatData(void* _this, void* reader)
{
    typedef void (__thiscall* LoadNewFormatDataProto)(void*, void*);

    LoadNewFormatDataProto callback = (LoadNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_LOAD_NEW_FORMAT_DATA);
    if (callback)
    {
        callback(_this, reader);
    }
}