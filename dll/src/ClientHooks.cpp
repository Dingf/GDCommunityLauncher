#include <filesystem>
#include <shlobj.h>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "HookManager.h"
#include "Client.h"
#include "Log.h"

const char* HandleGetVersion(void* arg1)
{
    Client& client = Client::GetInstance();
    return client.GetVersionInfoText().c_str();
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
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            char pathBuffer[260];
            if (!SHGetSpecialFolderPath(NULL, pathBuffer, CSIDL_PERSONAL, FALSE))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Call to SHGetSpecialFolderPath() failed.");
                return;
            }

            const wchar_t * playerName = GameAPI::GetPlayerName(mainPlayer);
            if (playerName == nullptr)
                return;

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
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            char pathBuffer[260];
            if (!SHGetSpecialFolderPath(NULL, pathBuffer, CSIDL_PERSONAL, FALSE))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Call to SHGetSpecialFolderPath() failed.");
                return;
            }

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

void HandleRender(void* arg1)
{
    typedef void(__thiscall* RenderProto)(void*);
    
    RenderProto callback = (RenderProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    if (callback)
    {
        callback(arg1);

        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (!GameAPI::IsGameLoading()) && ((std::string(modName) == "GrimLeagueS02_HC") || (std::string(modName) == "GrimLeagueS03")))
        {
            Client& client = Client::GetInstance();
            const std::wstring& text = client.GetLeagueInfoText();
            PULONG_PTR font = EngineAPI::LoadFontDirect("fonts/nevisshadow.fnt");
            EngineAPI::RenderText2D(10, 29, EngineAPI::Color::TAN, text.c_str(), font, 19, EngineAPI::GRAPHICS_X_ALIGN_LEFT, EngineAPI::GRAPHICS_Y_ALIGN_TOP, 0, 0);

            //TODO: Delete me, this is just testing code
            client.SetRank((client.GetRank() + 1) % 100 + 1);
            client.SetPoints(client.GetPoints() + 1);
        }
    }
}

// void World::Load(World* this, const char* mapName, bool unk1, bool modded)
bool HandleLoadWorld(void* arg1, const char* arg2, bool arg3, bool arg4)
{
    typedef bool(__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        bool result = callback(arg1, arg2, arg3, arg4);

        // Insert any code that needs to happen on map load here

        return result;
    }
    return false;
}

bool HandleKeyEvent(void* arg1, EngineAPI::KeyButtonEvent& arg2)
{
    typedef bool(__thiscall* TestProto)(void*, EngineAPI::KeyButtonEvent&);

    TestProto callback = (TestProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        // Disable the tilde key to prevent console access
        if (arg2._keyCode == EngineAPI::KEY_TILDE)
            return true;
        else
            return callback(arg1, arg2);
    }
    return false;
}


bool Client::SetupClientHooks()
{
    if (!HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION, &HandleGetVersion) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER, &HandleRender) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD, &HandleLoadWorld) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT, &HandleKeyEvent) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH, &HandleSaveTransferStash))
        return false;

    UpdateVersionInfoText();

    return true;
}

void Client::CleanupClientHooks()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);

    //TODO: (In another function perhaps) scan the user save directory for all characters and upload them to the server
}