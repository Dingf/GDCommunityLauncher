#include <filesystem>
#include "HookManager.h"
#include "UpdateThread.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "Client.h"
#include "Character.h"
#include "SharedStash.h"
#include "Log.h"

namespace
{

std::shared_ptr<UpdateThread> characterUpdateThread;
std::shared_ptr<UpdateThread> stashUpdateThread;

}

const char* HandleGetVersion(void* _this)
{
    Client& client = Client::GetInstance();
    return client.GetVersionInfoText().c_str();
}

void UpdateCharacterData()
{
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
    std::string baseFolder = GameAPI::GetBaseFolder();
    const wchar_t* playerName = GameAPI::GetPlayerName(mainPlayer);
    if ((playerName == nullptr) || (baseFolder.empty()))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine character save location.");
        return;
    }

    std::filesystem::path characterPath = std::filesystem::path(baseFolder) / "save" / "user" / "_";
    characterPath += playerName;
    characterPath /= "player.gdc";

    if (!std::filesystem::is_regular_file(characterPath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find saved character data. Make sure that cloud saving is disabled.");
        return;
    }

    Character characterData;
    if (!characterData.ReadFromFile(characterPath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
        return;
    }

    web::json::value characterJSON = characterData.ToJSON();

    //TODO: Send the character JSON data to the server
}

void HandleSaveNewFormatData(void* _this, void* writer)
{
    typedef void(__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        callback(_this, writer);

        //TODO: Change mod name for S3
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            characterUpdateThread->Update(1000);
        }
    }
}

void UpdateStashData()
{
    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine shared stash save location.");
        return;
    }

    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
    std::filesystem::path stashPath = std::filesystem::path(GameAPI::GetBaseFolder()) / "save" / modName;
    if (GameAPI::IsPlayerHardcore(mainPlayer))
        stashPath /= "transfer.gsh";
    else
        stashPath /= "transfer.gst";

    if (!std::filesystem::is_regular_file(stashPath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find shared stash data. Make sure that cloud saving is disabled.");
        return;
    }

    SharedStash stashData;
    if (!stashData.ReadFromFile(stashPath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash data.");
        return;
    }

    web::json::value stashJSON = stashData.ToJSON();

    //TODO: Send the shared stash JSON data to the server
}

void HandleSaveTransferStash(void* _this)
{
    typedef void(__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        callback(_this);

        //TODO: Change mod name for S3
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (std::string(modName) == "GrimLeagueS02_HC"))
        {
            stashUpdateThread->Update(5000);
        }
    }
}

void HandleRender(void* _this)
{
    typedef void(__thiscall* RenderProto)(void*);
    
    RenderProto callback = (RenderProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    if (callback)
    {
        callback(_this);

        // Insert any code that needs to happen every frame during rendering here. Note that this will occur after all other entities have been rendered
    }
}

bool HandleLoadWorld(void* _this, const char* mapName, bool unk1, bool modded)
{
    typedef bool(__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        bool result = callback(_this, mapName, unk1, modded);

        // Insert any code that needs to happen on map load here

        return result;
    }
    return false;
}

bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event)
{
    typedef bool(__thiscall* HandleKeyEventProto)(void*, EngineAPI::KeyButtonEvent&);

    HandleKeyEventProto callback = (HandleKeyEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        // TODO: Re-enable for S3
        // Disable the tilde key to prevent console access
        //if (arg2._keyCode == EngineAPI::KEY_TILDE)
        //    return true;
        //else
            return callback(_this, event);
    }
    return false;
}

void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int layout)
{
    typedef void(__thiscall* RenderTextStyled2DProto)(void*, const EngineAPI::Rect&, const wchar_t*, const std::string&, float, EngineAPI::GraphicsXAlign, EngineAPI::GraphicsYAlign, int);

    RenderTextStyled2DProto callback = (RenderTextStyled2DProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    if (callback)
    {
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

        // If the player is in-game on the S3 mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x == 10.0f) && (rect._y == 10.0f) && (modName) && (mainPlayer) && ((std::string(modName) == "GrimLeagueS02_HC") || (std::string(modName) == "GrimLeagueS03")))
        {
            Client& client = Client::GetInstance();
            std::wstring textString(text);
            if (textString.empty())
                textString += L"Normal";
            textString += client.GetLeagueInfoText();

            callback(_this, rect, textString.c_str(), style, unk, xAlign, yAlign, layout);

            //TODO: Delete me, this is just testing code
            client.SetRank((client.GetRank() + 1) % 100 + 1);
            client.SetPoints(client.GetPoints() + 1);
        }
        else
        {
            callback(_this, rect, text, style, unk, xAlign, yAlign, layout);
        }
    }
}


bool Client::SetupClientHooks()
{
    if (!HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION, &HandleGetVersion) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER, &HandleRender) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD, &HandleLoadWorld) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT, &HandleKeyEvent) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D, &HandleRenderStyledText2D) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH, &HandleSaveTransferStash))
        return false;

    UpdateVersionInfoText();

    characterUpdateThread = std::make_shared<UpdateThread>(&UpdateCharacterData, 1000);
    stashUpdateThread = std::make_shared<UpdateThread>(&UpdateStashData, 1000);

    return true;
}

void Client::CleanupClientHooks()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);

    characterUpdateThread->Stop();
    stashUpdateThread->Stop();

    //TODO: (In another function perhaps) scan the user save directory for all characters and upload them to the server
}