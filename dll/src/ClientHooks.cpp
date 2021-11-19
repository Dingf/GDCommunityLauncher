#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <cpprest/http_client.h>
#include "HookManager.h"
#include "UpdateThread.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "Client.h"
#include "ItemDatabaseValues.h"
#include "ItemDatabase.h"
#include "Character.h"
#include "SharedStash.h"
#include "Quest.h"
#include "Log.h"
#include "URI.h"

namespace
{

std::shared_ptr<UpdateThread<std::wstring, bool>> characterUpdateThread;
std::shared_ptr<UpdateThread<Client*>> clientTokenUpdateThread;

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

void Client::PostRefreshToken()
{
    URI endpoint = URI(GetHostName()) / "api" / "Account" / "refresh-token";
    web::http::client::http_client httpClient((utility::string_t)endpoint);

    web::json::value requestBody;
    requestBody[U("refreshToken")] = JSONString(GetRefreshToken());

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    httpClient.request(request).then([this](web::http::http_response response)
    {
        try
        {
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
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to refresh token: %", ex.what());
        }
    });
}

void UpdateClientTokens(Client* client)
{
    client->PostRefreshToken();
}

uint32_t GetCharacterID(std::wstring playerName)
{
    Client& client = Client::GetInstance();
    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character" / playerName;
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    pplx::task<uint32_t> task = httpClient.request(request).then([&client](web::http::http_response response)
    {
        try
        {
            switch (response.status_code())
            {
                case web::http::status_codes::OK:
                {
                    web::json::value responseBody = response.extract_json().get();
                    web::json::value characterID = responseBody[U("participantCharacterId")];
                    return (uint32_t)characterID.as_integer();
                }
                case web::http::status_codes::NoContent:
                {
                    return (uint32_t)0;
                }
                default:
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve character ID: %", ex.what());
        }
        return (uint32_t)0;
    });

    return task.get();
}

void UpdateSeasonStanding()
{
    Client& client = Client::GetInstance();

    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "standing";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    httpClient.request(request).then([&client](web::http::http_response response)
    {
        try
        {
            switch (response.status_code())
            {
                case web::http::status_codes::OK:
                {
                    web::json::value responseBody = response.extract_json().get();
                    web::json::value pointTotal = responseBody[U("pointTotal")];
                    web::json::value rank = responseBody[U("rank")];

                    client.SetRank(rank.as_integer());
                    client.SetPoints(pointTotal.as_integer());
                    break;
                }
                default:
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update season standing: %", ex.what());
        }
    });
}

std::filesystem::path GetPlayerCharacterPath(std::wstring playerName)
{
    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine the base Grim Dawn save location");
        return {};
    }

    std::filesystem::path characterPath = std::filesystem::path(baseFolder) / "save" / "user" / "_";
    characterPath += playerName;

    return characterPath;
}

void UpdateCharacterData(std::wstring playerName, bool async)
{
    Client& client = Client::GetInstance();
    uint32_t characterID = GetCharacterID(playerName);

    std::filesystem::path characterSavePath = GetPlayerCharacterPath(playerName) / "player.gdc";
    std::filesystem::path characterQuestPath = GetPlayerCharacterPath(playerName) / "maps_world001.map";

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
    characterInfo[U("maxDifficulty")] = characterJSON[U("InfoBlock")][U("MaxDifficulty")];
    characterInfo[U("currentDifficulty")] = characterJSON[U("InfoBlock")][U("CurrentDifficulty")].as_integer() & 0x7F;
    characterInfo[U("deathCount")] = characterJSON[U("StatsBlock")][U("Deaths")];
    characterInfo[U("timePlayed")] = characterJSON[U("StatsBlock")][U("PlayedTime")];
    characterInfo[U("physique")] = characterJSON[U("AttributesBlock")][U("Physique")];
    characterInfo[U("cunning")] = characterJSON[U("AttributesBlock")][U("Cunning")];
    characterInfo[U("spirit")] = characterJSON[U("AttributesBlock")][U("Spirit")];
    characterInfo[U("devotionPoints")] = characterJSON[U("AttributesBlock")][U("TotalDevotionPoints")];
    characterInfo[U("health")] = characterJSON[U("AttributesBlock")][U("Health")];
    characterInfo[U("energy")] = characterJSON[U("AttributesBlock")][U("Energy")];

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

    if (async)
    {
        httpClient.request(request).then([](web::http::http_response response)
        {
            try
            {
                if (response.status_code() == web::http::status_codes::OK)
                {
                    UpdateSeasonStanding();
                }
                else
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
            }
        });
    }
    else
    {
        try
        {
            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                UpdateSeasonStanding();
            }
            else
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
        }
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
        const SeasonInfo* seasonInfo = client.GetActiveSeason();
        if ((modName) && (mainPlayer) && (!EngineAPI::IsMultiplayer()) && (client.IsParticipatingInSeason()))
        {
            characterUpdateThread->Update(5000, GameAPI::GetPlayerName(mainPlayer), true);
        }
    }
}

std::filesystem::path GetSharedStashPath(const char* modName, bool hardcore)
{
    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine the base Grim Dawn save location");
        return {};
    }

    std::filesystem::path stashPath = std::filesystem::path(baseFolder) / "save";
    if (modName)
        stashPath /= modName;

    if (hardcore)
        stashPath /= "transfer.gsh";
    else
        stashPath /= "transfer.gst";

    return stashPath;
}

void PostTransferStashUpload()
{
    Client& client = Client::GetInstance();
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
    {
        std::filesystem::path stashPath = GetSharedStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer));

        SharedStash stashData;
        if (!stashData.ReadFromFile(stashPath))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash data. Make sure that cloud saving is diabled.");
            return;
        }

        web::json::value requestBody = web::json::value::array();

        web::json::value stashJSON = stashData.ToJSON();
        web::json::array stashTabs = stashJSON[U("Tabs")].as_array();
        if (stashTabs.size() >= 6)
        {
            web::json::value transferTab = stashTabs[5];
            web::json::array transferTabItems = transferTab[U("Items")].as_array();

            uint32_t index = 0;
            for (auto it = transferTabItems.begin(); it != transferTabItems.end(); ++it)
            {
                requestBody[index] = *it;
                requestBody[index].erase(U("Unknown1"));
                requestBody[index].erase(U("Unknown2"));
                requestBody[index].erase(U("X"));
                requestBody[index].erase(U("Y"));
                index++;
            }
        }

        URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "stash";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            try
            {
                if (response.status_code() == web::http::status_codes::OK)
                {
                    //TODO: Save the stash minus the items that were transferred
                }
                else
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
            }
        })
        .wait();
    }
}

std::time_t GetStashLastModifiedTime()
{
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
    if (mainPlayer)
    {
        std::filesystem::path stashPath = GetSharedStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer));
        if (!stashPath.empty())
        {
            std::filesystem::file_time_type lastModifiedTime = std::filesystem::last_write_time(stashPath);
            return std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastModifiedTime - decltype(lastModifiedTime)::clock::now() + std::chrono::system_clock::now()));
        }
    }
    return std::time_t(-1);
}

void PostSharedStashUpdateTimes(std::time_t prevModifiedTime)
{
    Client& client = Client::GetInstance();
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
    {
        web::json::value requestBody;
        requestBody[U("lastModifiedOn")] = prevModifiedTime;
        requestBody[U("sharedStashModifiedOn")] = GetStashLastModifiedTime();

        URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "shared-stash";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            try
            {
                if (response.status_code() != web::http::status_codes::OK)
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
            }
        });
    }
}

void HandleSaveTransferStash(void* _this)
{
    typedef void(__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        std::time_t prevModifiedTime = GetStashLastModifiedTime();
        callback(_this);

        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
        {
            pplx::create_task([prevModifiedTime]()
            {
                //PostTransferStashUpload();
                PostSharedStashUpdateTimes(prevModifiedTime);
            });
        }
    }
}

void HandleLoadTransferStash(void* _this)
{
    typedef void(__thiscall* LoadPlayerTransferProto)(void*);
    LoadPlayerTransferProto callback = (LoadPlayerTransferProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);
    if (callback)
    {
        callback(_this);

        // TODO: Download the items to be transferred from the server and save the stash
        //       This may affect the file last modified time as well, so we need to store that before making any changes
    }
}

void HandleSetMainPlayer(void* _this, uint32_t unk1)
{
    typedef void(__thiscall* SetMainPlayerProto)(void*, uint32_t);

    SetMainPlayerProto callback = (SetMainPlayerProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER);
    if (callback)
    {
        callback(_this, unk1);

        Client& client = Client::GetInstance();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((mainPlayer) && (seasonInfo))
        {
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
            bool hasParticipationToken = GameAPI::HasToken(mainPlayer, seasonInfo->_participationToken);

            if (GameAPI::GetGameDifficulty() != GameAPI::GAME_DIFFICULTY_NORMAL)
            {
                Quest questData;
                if (questData.ReadFromFile(GetPlayerCharacterPath(playerName) / "maps_world001.map" / "Normal" / "quests.gdd"))
                {
                    web::json::value questJSON = questData.ToJSON();
                    web::json::array tokensArray = questJSON[U("Tokens")][U("Tokens")].as_array();

                    uint32_t index = 0;
                    for (auto it2 = tokensArray.begin(); it2 != tokensArray.end(); ++it2)
                    {
                        std::string token = JSONString(it2->serialize());
                        token = std::string(token.begin() + 1, token.end() - 1);    // Trim quotes before storing as a JSON string
                        if (token == seasonInfo->_participationToken)
                        {
                            hasParticipationToken = true;
                            break;
                        }
                    }
                }
            }

            client.SetActiveCharacter(playerName, hasParticipationToken);
            if (hasParticipationToken)
            {
                UpdateCharacterData(playerName, false);
            }
        }
    }
}

void HandleBestowToken(void* _this, void* token)
{
    typedef void(__thiscall* BestowTokenProto)(void*, void*);

    BestowTokenProto callback = (BestowTokenProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN);
    if (callback)
    {
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((modName) && (mainPlayer) && (seasonInfo))
        {
            // Sometimes the token appears to be garbage memory, so check the flags to make sure it's valid
            uint32_t tokenFlags = *((uint32_t*)token + 4);
            if (tokenFlags <= 0xFF)
            {
                std::string tokenString = std::string(*((const char**)token + 1));
                if ((EngineAPI::IsMultiplayer()) && (tokenString != "Received_Start_Items"))
                {
                    // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
                    return;
                }
                else if (tokenString == seasonInfo->_participationToken)
                {
                    std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
                    client.SetActiveCharacter(playerName, true);
                    UpdateCharacterData(playerName, true);
                }
                else if ((tokenString.find("GDL_", 0) == 0) && (client.IsParticipatingInSeason()))
                {
                    // Otherwise if it's a season token, pass it along to the server and update the points/rank
                    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "quest-tag" / tokenString;
                    web::http::client::http_client httpClient((utility::string_t)endpoint);
                    web::http::http_request request(web::http::methods::POST);

                    web::json::value requestBody;
                    requestBody[U("level")] = EngineAPI::GetPlayerLevel();
                    requestBody[U("currentDifficulty")] = GameAPI::GetGameDifficulty();
                    requestBody[U("maxDifficulty")] = GameAPI::GetPlayerMaxDifficulty(mainPlayer);
                    request.set_body(requestBody);

                    std::string bearerToken = "Bearer " + client.GetAuthToken();
                    request.headers().add(U("Authorization"), bearerToken.c_str());

                    httpClient.request(request).then([](web::http::http_response response)
                    {
                        try
                        {
                            if (response.status_code() == web::http::status_codes::OK)
                            {
                                UpdateSeasonStanding();
                            }
                            else
                            {
                                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                            }
                        }
                        catch (const std::exception& ex)
                        {
                            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update quest tag: %", ex.what());
                        }
                    });
                }
            }
        }

        callback(_this, token);
    }
}

void HandleRender(void* _this)
{
    typedef void(__thiscall* RenderProto)(void*);
    
    RenderProto callback = (RenderProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    if (callback)
    {
        callback(_this);

        // Terminate the process immediately if GI is detected
        if (GetModuleHandle(TEXT("GrimInternalsDll64.dll")))
        {
            ExitProcess(EXIT_SUCCESS);
        }
    }
}

bool HandleLoadWorld(void* _this, const char* mapName, bool unk1, bool modded)
{
    typedef bool(__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        bool result = callback(_this, mapName, unk1, modded);
        if (result)
        {
            Client& client = Client::GetInstance();
            ItemDatabase& database = ItemDatabase::GetInstance();
            const char* modName = EngineAPI::GetModName();

            // Check the map name to make sure that we are not in the main menu when setting the active season
            if ((modName) && (mapName) && (std::string(mapName) != "levels/mainmenu/mainmenu.map"))
            {
                client.SetActiveSeason(modName, EngineAPI::GetHardcore());

                // Attempt to register the user for the active season
                const SeasonInfo* seasonInfo = client.GetActiveSeason();
                if (seasonInfo)
                {
                    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / std::to_string(seasonInfo->_seasonID) / "add-participant" / client.GetUsername();
                    web::http::client::http_client httpClient((utility::string_t)endpoint);
                    web::http::http_request request(web::http::methods::POST);

                    std::string bearerToken = "Bearer " + client.GetAuthToken();
                    request.headers().add(U("Authorization"), bearerToken.c_str());

                    httpClient.request(request).then([&client, seasonInfo](web::http::http_response response)
                    {
                        try
                        {
                            web::http::status_code status = response.status_code();
                            if (status == web::http::status_codes::OK)
                            {
                                web::json::value responseBody = response.extract_json().get();
                                web::json::value participantID = responseBody[U("seasonParticipantId")];
                                client.SetParticipantID(participantID.as_integer());
                                UpdateSeasonStanding();
                            }
                            else
                            {
                                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                            }
                        }
                        catch (const std::exception& ex)
                        {
                            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to register for season %: %", seasonInfo->_seasonID, ex.what());
                        }
                    })
                    .wait();
                }
            }
            else if (!database.IsLoaded())
            {
                HINSTANCE launcherDLL = GetModuleHandle(TEXT("GDCommunityLauncher.dll"));
                HRSRC res = FindResource(launcherDLL, MAKEINTRESOURCE(IDR_ITEMDB), RT_RCDATA);
                if (!res)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load item database from DLL. Item and stash functions will not work properly!");
                }
                else
                {
                    HGLOBAL handle = LoadResource(launcherDLL, res);
                    DWORD size = SizeofResource(launcherDLL, res);
                    char* data = (char*)LockResource(handle);
                    database.Load(data, size);
                    FreeResource(handle);
                }
            }
        }
        return result;
    }
    return false;
}

void HandleUnloadWorld(void* _this)
{
    typedef void(__thiscall* UnloadWorldProto)(void*);

    UnloadWorldProto callback = (UnloadWorldProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD);
    if (callback)
    {
        callback(_this);

        Client& client = Client::GetInstance();
        const std::wstring& characterName = client.GetActiveCharacterName();
        if ((!EngineAPI::IsMultiplayer()) && (client.IsParticipatingInSeason()) && (!characterName.empty()))
        {
            UpdateCharacterData(characterName, false);
            client.SetActiveCharacter({}, false);
        }
    }
}

bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event)
{
    typedef bool(__thiscall* HandleKeyEventProto)(void*, EngineAPI::KeyButtonEvent&);

    HandleKeyEventProto callback = (HandleKeyEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

        if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()) && (event._keyCode == EngineAPI::KEY_TILDE))
        {
            // Disable the tilde key to prevent console access
            return true;
        }
        else
        {
            return callback(_this, event);
        }
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

        // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x >= 7.0f) && (rect._y >= 7.0f) && (rect._x <= 18.0f) && (rect._y <= 18.0f) && (rect._x == rect._y) && (modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
        {
            std::wstring textString(text);
            if (textString.empty())
                textString += L"Normal";
            textString += client.GetLeagueInfoText();

            callback(_this, rect, textString.c_str(), style, unk, xAlign, yAlign, layout);
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
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER, &HandleSetMainPlayer) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH, &HandleSaveTransferStash) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH, &HandleLoadTransferStash) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN, &HandleBestowToken) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD, &HandleUnloadWorld))
        return false;

    UpdateVersionInfoText();

    characterUpdateThread = std::make_shared<UpdateThread<std::wstring, bool>>(&UpdateCharacterData);
    clientTokenUpdateThread = std::make_shared<UpdateThread<Client*>>(&UpdateClientTokens, 1000, 1800000);

    // Refresh the client tokens immediately and then every 30 minutes after
    Client& client = Client::GetInstance();
    clientTokenUpdateThread->Update(0, &client);

    return true;
}

void Client::CleanupClientHooks()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD);

    characterUpdateThread->Stop();
    clientTokenUpdateThread->Stop();

    //TODO: (In another function perhaps) scan the user save directory for all characters and upload them to the server
}