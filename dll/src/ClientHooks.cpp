#include <filesystem>
#include <cpprest/http_client.h>
#include "HookManager.h"
#include "UpdateThread.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "Client.h"
#include "Character.h"
#include "SharedStash.h"
#include "Quest.h"
#include "Log.h"
#include "URI.h"

namespace
{

std::shared_ptr<UpdateThread<std::wstring>> characterUpdateThread;
std::shared_ptr<UpdateThread<std::string, bool>> stashUpdateThread;

}

const std::unordered_map<std::string, const utility::char_t*> difficultyTagLookup =
{
    { "Normal",   U("normalQuestTags") },
    { "Elite",    U("eliteQuestTags") },
    { "Ultimate", U("ultimateQuestTags") },
};

const char* HandleGetVersion(void* _this)
{
    Client& client = Client::GetInstance();
    return client.GetVersionInfoText().c_str();
}

void Client::SendRefreshToken()
{
    URI endpoint = URI(GetHostName()) / "api" / "Account" / "refresh-token";
    web::http::client::http_client httpClient((utility::string_t)endpoint);

    web::json::value requestBody;
    requestBody[U("refreshToken")] = JSONString(GetRefreshToken());

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::value authTokenValue = responseBody[U("access_token")];
            web::json::value refreshTokenValue = responseBody[U("refresh_token")];
            if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
            {
                _data._authToken = JSONString(authTokenValue.as_string());
                _data._refreshToken = JSONString(refreshTokenValue.as_string());
            }
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to refresh token: Server responded with status code %", response.status_code());
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to refresh token: %", ex.what());
    }
}

uint32_t GetCharacterID(std::wstring playerName)
{
    Client& client = Client::GetInstance();
    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character" / playerName;
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::value characterID = responseBody[U("participantCharacterId")];
                return characterID.as_integer();
            }
            case web::http::status_codes::NoContent:
                return 0;
            default:
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve character data: Server responded with status code %", response.status_code());
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve character data: %", ex.what());
    }
    return 0;
}

void UpdateCharacterData(std::wstring playerName)
{
    Client& client = Client::GetInstance();
    client.SendRefreshToken();

    uint32_t characterID = GetCharacterID(playerName);

    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine save location.");
        return;
    }

    std::filesystem::path characterPath = std::filesystem::path(baseFolder) / "save" / "user" / "_";
    characterPath += playerName;

    std::filesystem::path characterSavePath = characterPath / "player.gdc";
    std::filesystem::path characterQuestPath = characterPath / "Levels_world001.map";

    Character characterData;
    if (!characterData.ReadFromFile(characterSavePath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data. Make sure that cloud saving is diabled.");
        return;
    }

    web::json::value characterJSON = characterData.ToJSON();
    web::json::value characterInfo = web::json::value::object();

    characterInfo[U("name")] = characterJSON[U("HeaderBlock")][U("Name")];
    characterInfo[U("level")] = characterJSON[U("HeaderBlock")][U("Level")];
    characterInfo[U("className")] = characterJSON[U("HeaderBlock")][U("ClassName")];
    characterInfo[U("hardcore")] = characterJSON[U("HeaderBlock")][U("Hardcore")];

    web::json::value questInfo = web::json::value::object();
    for (auto it = difficultyTagLookup.begin(); it != difficultyTagLookup.end(); ++it)
    {
        Quest questData;
        questInfo[it->second] = web::json::value::array();
        if (questData.ReadFromFile(characterQuestPath / it->first / "quests.gdd"))
        {
            web::json::value questJSON = questData.ToJSON();
            web::json::array tokensArray = questJSON[U("Tokens")][U("Tokens")].as_array();

            uint32_t index = 0;
            for (auto it2 = tokensArray.begin(); it2 != tokensArray.end(); ++it2)
            {
                std::string token = JSONString(it2->serialize());
                token = std::string(token.begin() + 1, token.end() - 1);    // Trim quotes before storing as a JSON string
                if (token.find("GDL_", 0) == 0)
                {
                    questInfo[it->second][index++] = JSONString(token);
                }
            }
        }
    }

    web::json::value requestBody;
    requestBody[U("characterData")] = web::json::value::object();
    requestBody[U("characterData")][U("characterInfo")] = characterInfo;
    requestBody[U("characterData")][U("questInfo")] = questInfo;
    requestBody[U("seasonParticipantId")] = client.GetParticipantID();
    requestBody[U("participantCharacterId")] = characterID;

    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character";

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() != web::http::status_codes::OK)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: Server responded with status code %", response.status_code());
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
    }
}

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
        if ((modName) && (mainPlayer) && (std::string(modName) == client.GetSeasonModName()))
        {
            characterUpdateThread->Update(5000, GameAPI::GetPlayerName(mainPlayer));
        }
    }
}

void UpdateStashData(std::string modName, bool hardcore)
{
    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine save location.");
        return;
    }

    std::filesystem::path stashPath = std::filesystem::path(GameAPI::GetBaseFolder()) / "save" / modName;
    if (hardcore)
        stashPath /= "transfer.gsh";
    else
        stashPath /= "transfer.gst";

    SharedStash stashData;
    if (!stashData.ReadFromFile(stashPath))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash data. Make sure that cloud saving is diabled.");
        return;
    }

    web::json::value stashJSON = stashData.ToJSON();

    //TODO: Trim and send the shared stash JSON data to the server
}

void HandleSaveTransferStash(void* _this)
{
    typedef void(__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        callback(_this);

        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (std::string(modName) == client.GetSeasonModName()))
        {
            stashUpdateThread->Update(5000, modName, GameAPI::IsPlayerHardcore(mainPlayer));
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
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

        // If the player is in-game on the S3 mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x == 10.0f) && (rect._y == 10.0f) && (modName) && (mainPlayer) && (std::string(modName) == client.GetSeasonModName()))
        {
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

    characterUpdateThread = std::make_shared<UpdateThread<std::wstring>>(&UpdateCharacterData, 1000);
    stashUpdateThread = std::make_shared<UpdateThread<std::string, bool>>(&UpdateStashData, 1000);

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