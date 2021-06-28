#include <filesystem>
#include <shlobj.h>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "HookManager.h"
#include "Client.h"
#include "Log.h"

std::string versionString;

const char* HandleGetVersion(void* arg1)
{
    typedef const char*(__thiscall* GetVersionProto)(void*);

    versionString.clear();

    Client& client = Client::GetInstance();
    GetVersionProto callback = (GetVersionProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    if (callback)
    {
        const char* result = callback(arg1);
        versionString.append(result);
        versionString.append("\n{^F}GrimLeague S3 (");
        versionString.append(client.GetName());
        versionString.append(")");
    }

    return versionString.c_str();
}

void HandleSaveNewFormatData(void* arg1, void* arg2)
{
    typedef void(__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        callback(arg1, arg2);

        //TODO: Change mod name for S3
        const char* modName = EngineAPI::GetModName();
        if ((modName) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            char pathBuffer[260];
            if (!SHGetSpecialFolderPath(NULL, pathBuffer, CSIDL_PERSONAL, FALSE))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Call to SHGetSpecialFolderPath() failed.");
                return;
            }

            std::wstring playerName = GameAPI::GetPlayerName(GameAPI::GetMainPlayer());
            std::wstring playerFolderName = L"_";
            playerFolderName += playerName;

            std::filesystem::path basePath = pathBuffer;
            std::filesystem::path characterPath = basePath / "My Games" / "Grim Dawn" / "save" / "user" / playerFolderName / "player.gdc";

            if (!std::filesystem::is_regular_file(characterPath))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find saved character data. Make sure that cloud saving is disabled.");
                return;
            }

            //TODO: Load the character data and send it to the server
            //TODO: Also, since the file may be updated frequently when putting in skill/devotion points, we need to wait like ~5s. During that 5s,
            //      any further saves should not create a new request but instead extend the timer on the old one. This way, the server doesn't get
            //      flooded with save messages.
            //      This should also allow for enough time to properly save the character/quest data on the filesystem
        }
    }
}

void HandleSaveTransferStash(void* arg1)
{
    typedef void(__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        callback(arg1);

        //TODO: Change mod name for S3
        const char* modName = EngineAPI::GetModName();
        if ((modName) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            char pathBuffer[260];
            if (!SHGetSpecialFolderPath(NULL, pathBuffer, CSIDL_PERSONAL, FALSE))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Call to SHGetSpecialFolderPath() failed.");
                return;
            }

            PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
            bool hardcore = GameAPI::IsPlayerHardcore(mainPlayer);

            std::filesystem::path stashPath = pathBuffer;
            stashPath = stashPath / "My Games" / "Grim Dawn" / "save" / modName;
            if (hardcore)
                stashPath /= "transfer.gsh";
            else
                stashPath /= "transfer.gst";

            if (!std::filesystem::is_regular_file(stashPath))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find saved shared stash data. Make sure that cloud saving is disabled.");
                return;
            }

            //TODO: Load the shared stash data and send it to the server
        }
    }
}

bool Client::SetupClientHooks()
{
    if (!HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION, &HandleGetVersion) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH, &HandleSaveTransferStash))
        return false;

    return true;
}

void Client::CleanupClientHooks()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);

    //TODO: (In another function perhaps) scan the user save directory for all characters and upload them to the server
}