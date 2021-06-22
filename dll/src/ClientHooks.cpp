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

        std::string modName = EngineAPI::GetModName();
        //TODO: Change mod name for S3
        if (modName == "GrimLeagueS02_HC")
        {
            char pathBuffer[260];
            if (!SHGetSpecialFolderPath(NULL, pathBuffer, CSIDL_PERSONAL, FALSE))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Call to SHGetSpecialFolderPath() failed.");
                return;
            }

            std::wstring playerName = L"_";
            playerName += GameAPI::GetPlayerName(GameAPI::GetMainPlayer());

            std::filesystem::path basePath = pathBuffer;
            std::filesystem::path characterPath = basePath / "My Games" / "Grim Dawn" / "save" / "user" / playerName;
            std::filesystem::path characterSavePath = characterPath / "player.gdc";

            if (std::filesystem::is_directory(characterPath) && std::filesystem::is_regular_file(characterSavePath))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find character data for %. Make sure that cloud saving is disabled.", GameAPI::GetPlayerName(GameAPI::GetMainPlayer()));
                return;
            }

            //TODO: Load the character data and send it to the server
        }
    }
}

bool Client::SetupClientHooks()
{
    if (!HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION, &HandleGetVersion) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData))
        return false;

    return true;
}

void Client::CLeanupClientHooks()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
}