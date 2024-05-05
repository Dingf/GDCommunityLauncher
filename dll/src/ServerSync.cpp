#include <unordered_map>
#include <filesystem>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include "Client.h"
#include "EventManager.h"
#include "ThreadManager.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ServerSync.h"
#include "Character.h"
#include "SharedStash.h"
#include "Version.h"
#include "Log.h"

const std::unordered_map<GameAPI::Difficulty, const utility::char_t*> difficultyTagLookup =
{
    { GameAPI::GAME_DIFFICULTY_NORMAL,   U("normalQuestTags") },
    { GameAPI::GAME_DIFFICULTY_ELITE,    U("eliteQuestTags") },
    { GameAPI::GAME_DIFFICULTY_ULTIMATE, U("ultimateQuestTags") },
};

std::string GetMultipartBoundary()
{
    return "----GrimDawnCommunityLauncherV" + std::string(GDCL_VERSION);
}

std::string GetMultipartFileData(const std::string& name, const std::filesystem::path& filePath)
{
    std::ifstream inputFile(filePath, std::ios::binary | std::ios::in);
    if (inputFile)
    {
        std::ostringstream inputData;
        inputData << inputFile.rdbuf();
        inputFile.close();

        std::stringstream content;
        content << "--" << GetMultipartBoundary() << "\r\n";
        content << "Content-Disposition: form-data; name=\"" << name << "\"; filename=\"" << filePath.filename().string() << "\"\r\n"
                << "Content-Type: application/octet-stream\r\n\r\n"
                << inputData.str() << "\r\n";

        return content.str();
    }
    return {};
}

std::string GetMultipartBufferData(const std::string& name, const std::filesystem::path& filename, uint8_t* buffer, size_t size)
{
    std::ostringstream inputData;
    inputData.write((const char*)buffer, size);

    std::stringstream content;
    content << "--" << GetMultipartBoundary() << "\r\n";
    content << "Content-Disposition: form-data; name=\"" << name << "\"; filename=\"" << filename.string() << "\"\r\n"
        << "Content-Type: application/octet-stream\r\n\r\n"
        << inputData.str() << "\r\n";

    return content.str();
}

std::string GetMultipartJSONData(const std::string& name, const web::json::value& data)
{
    std::stringstream content;
    content << "--" << GetMultipartBoundary() << "\r\n"
            << "Content-Disposition: form-data; name=\"" << name << "\"\r\n"
            << "Content-Type: application/json\r\n\r\n"
            << (std::string)JSONString(data.serialize()) << "\r\n";

    return content.str();
}

ServerSync::ServerSync()
{
    EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,            &ServerSync::OnShutdownEvent);
    EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_READ,    &ServerSync::OnDirectReadEvent);
    EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_WRITE,   &ServerSync::OnDirectWriteEvent);
    EventManager::Subscribe(GDCL_EVENT_ADD_SAVE_JOB,        &ServerSync::OnAddSaveJob);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,      &ServerSync::OnWorldPreLoadEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_POST_LOAD,     &ServerSync::OnWorldPostLoadEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD,    &ServerSync::OnWorldPreUnloadEvent);
    EventManager::Subscribe(GDCL_EVENT_SET_MAIN_PLAYER,     &ServerSync::OnSetMainPlayerEvent);
    EventManager::Subscribe(GDCL_EVENT_TRANSFER_POST_LOAD,  &ServerSync::OnTransferPostLoadEvent);
    EventManager::Subscribe(GDCL_EVENT_TRANSFER_PRE_SAVE,   &ServerSync::OnTransferPreSaveEvent);
    EventManager::Subscribe(GDCL_EVENT_CHARACTER_PRE_SAVE,  &ServerSync::OnCharacterPreSaveEvent);
    EventManager::Subscribe(GDCL_EVENT_CHARACTER_POST_SAVE, &ServerSync::OnCharacterPostSaveEvent);
}

ServerSync& ServerSync::GetInstance()
{
    static ServerSync instance;
    return instance;
}

uint32_t ServerSync::GetParticipantID(bool hardcore)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "profile";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::array seasonParticipants = responseBody[U("seasonParticipants")].as_array();
                for (auto it = seasonParticipants.begin(); it != seasonParticipants.end(); ++it)
                {
                    uint32_t participantID = it->at(U("seasonParticipantId")).as_integer();
                    uint32_t seasonType = it->at(U("season"))[U("seasonTypeId")].as_integer();
                    if (hardcore == (seasonType == SEASON_TYPE_HC_SSF))
                        return participantID;
                }
                return 0;
            }
            case web::http::status_codes::NoContent:
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Participant ID was not found for user \"%\"", client.GetUsername());
                return 0;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve participant ID: %", ex.what());
        return 0;
    }
}

uint32_t ServerSync::GetCharacterID(const std::wstring& playerName, uint32_t participantID)
{
    Client& client = Client::GetInstance();

    if (participantID == 0)
        participantID = client.GetParticipantID();

    CharacterIDRef ref = { playerName, participantID };
    if (_characterIDCache.count(ref) > 0)
    {
        return _characterIDCache[ref];
    }
    else
    {
        try
        {
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName;
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            switch (response.status_code())
            {
                case web::http::status_codes::OK:
                {
                    web::json::value responseBody = response.extract_json().get();
                    web::json::value characterID = responseBody[U("participantCharacterId")];
                    _characterIDCache[ref] = (uint32_t)characterID.as_integer();
                    return _characterIDCache[ref];
                }
                case web::http::status_codes::NoContent:
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Character ID not found for player \"%\"", (std::string)URI(playerName));
                    return 0;
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
            return 0;
        }
    }
}

FileMetadata ServerSync::GetServerCharacterMetadata(const std::wstring& playerName, uint32_t participantID)
{
    FileMetadata result;
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName;
        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                result._checksum = JSONString(responseBody[U("lastChecksum")].serialize());
                return result;
            }
            case web::http::status_codes::NoContent:
            {
                return result;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve character metadata: %", ex.what());
        return result;
    }
}

FileMetadata ServerSync::GetServerStashMetadata(uint32_t participantID)
{
    FileMetadata result;
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "shared-stash";
        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                result._checksum = JSONString(responseBody[U("lastChecksum")].serialize());
                return result;
            }
            case web::http::status_codes::NoContent:
            {
                return result;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve shared stash metadata: %", ex.what());
        return result;
    }
}

void ServerSync::RegisterSeasonParticipant(bool hardcore)
{
    try
    {
        Client& client = Client::GetInstance();
        client.SetActiveSeason(hardcore);
        if (const SeasonInfo* seasonInfo = client.GetActiveSeason())
        {
            URI endpoint = client.GetServerGameURL() / "Season" / std::to_string(seasonInfo->_seasonID) / "add-participant" / client.GetUsername();
            endpoint.AddParam("branch", client.GetBranchName());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::value participantID = responseBody[U("seasonParticipantId")];
                client.SetParticipantID(participantID.as_integer());
                client.UpdateSeasonStanding();
            }
            else
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        else
        {
            throw std::runtime_error("Client is not currently in an active season");
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to register season participant: %", ex.what());
    }
}

void ServerSync::UploadCharacterQuestData(uint32_t participantID)
{
    Client& client = Client::GetInstance();
    for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
    {
        std::string difficultyName = GetGameDifficultyName(difficulty);

        std::filesystem::path characterPath = GameAPI::GetPlayerFolder(_characterName);
        std::filesystem::path questFilePath = characterPath / "levels_world001.map" / difficultyName / "quests.gdd";

        if (std::filesystem::is_regular_file(questFilePath))
        {
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / _characterName / "questfile" / difficultyName;

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);

            std::string requestBody;
            requestBody += "\r\n";
            requestBody += GetMultipartFileData("file", questFilePath);
            requestBody += "--" + GetMultipartBoundary() + "--\r\n";

            request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            _backgroundTasks.run([endpoint, request]()
            {
                web::http::client::http_client httpClient((utility::string_t)endpoint);
                return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
                {
                    try
                    {
                        web::http::http_response response = responseTask.get();
                        if (response.status_code() != web::http::status_codes::OK)
                            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                    }
                    catch (std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload quest data: %", ex.what());
                    }
                });
            });
        }
    }
}

void BuildCharacterInfo(web::json::value& characterInfo, web::json::value& characterJSON)
{
    uint32_t currentDifficulty = characterJSON[U("InfoBlock")][U("CurrentDifficulty")].as_integer() & 0x7F;

    characterInfo[U("name")] = characterJSON[U("HeaderBlock")][U("Name")];
    characterInfo[U("level")] = characterJSON[U("HeaderBlock")][U("Level")];
    characterInfo[U("className")] = characterJSON[U("HeaderBlock")][U("ClassName")];
    characterInfo[U("hardcore")] = characterJSON[U("HeaderBlock")][U("Hardcore")];
    characterInfo[U("maxDifficulty")] = characterJSON[U("InfoBlock")][U("MaxDifficulty")];
    characterInfo[U("currentDifficulty")] = currentDifficulty;
    characterInfo[U("deathCount")] = characterJSON[U("StatsBlock")][U("Deaths")];
    characterInfo[U("timePlayed")] = characterJSON[U("StatsBlock")][U("PlayedTime")];
    characterInfo[U("physique")] = characterJSON[U("AttributesBlock")][U("Physique")];
    characterInfo[U("cunning")] = characterJSON[U("AttributesBlock")][U("Cunning")];
    characterInfo[U("spirit")] = characterJSON[U("AttributesBlock")][U("Spirit")];
    characterInfo[U("devotionPoints")] = characterJSON[U("AttributesBlock")][U("TotalDevotionPoints")];
    characterInfo[U("health")] = characterJSON[U("AttributesBlock")][U("Health")];
    characterInfo[U("energy")] = characterJSON[U("AttributesBlock")][U("Energy")];
    characterInfo[U("lastAttackedBy")] = characterJSON[U("StatsBlock")][U("PerDifficultyStats")][currentDifficulty][U("LastAttackedBy")];
}

void ServerSync::UploadCharacter(uint32_t participantID)
{
    Client& client = Client::GetInstance();

    if (participantID == 0)
        participantID = client.GetParticipantID();

    if ((client.IsPlayingSeasonOnline()) && (!_characterName.empty()))
    {
        std::filesystem::path characterPath = GameAPI::GetPlayerFolder(_characterName);
        std::filesystem::path characterSavePath = GameAPI::GetPlayerSaveFile(_characterName);

        // If using cloud saves, the character path won't exist so just return
        if (!std::filesystem::is_directory(characterPath))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Character path \"%\" was not found. Make sure that cloud saving is disabled.", characterPath.string());
            return;
        }

        std::vector<std::filesystem::path> characterQuestPaths;
        for (const auto& entry : std::filesystem::directory_iterator(characterPath))
        {
            if (entry.is_directory())
                characterQuestPaths.push_back(entry.path());
        }

        // Shouldn't happen too often, but wait to make sure that the save file is present and readable
        while (!std::ifstream(characterSavePath).good());

        Character characterData;
        if (!characterData.ReadFromFile(characterSavePath))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data. Make sure that cloud saving is disabled.");
            return;
        }

        web::json::value characterJSON = characterData.ToJSON();
        web::json::value characterInfo = web::json::value::object();

        BuildCharacterInfo(characterInfo, characterJSON);

        web::json::value questInfo = web::json::value::object();
        for (auto pair : difficultyTagLookup)
            questInfo[pair.second] = web::json::value::array();

        void* mainPlayer = GameAPI::GetMainPlayer();
        if (mainPlayer != nullptr)
        {
            for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
            {
                const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, difficulty);
                for (size_t i = 0, index = 0; i < tokens.size(); ++i)
                {
                    std::string tokenString = tokens[i];
                    for (char& c : tokenString)
                        c = std::tolower(c);

                    if (tokenString.find("gdl_", 0) == 0)
                        questInfo[difficultyTagLookup.at(difficulty)][index++] = JSONString(tokenString);
                }
            }
        }

        web::json::value requestJSON;
        requestJSON[U("characterData")] = web::json::value::object();
        requestJSON[U("characterData")][U("characterInfo")] = characterInfo;
        requestJSON[U("characterData")][U("questInfo")] = questInfo;
        requestJSON[U("seasonParticipantId")] = participantID;
        requestJSON[U("participantCharacterId")] = GetCharacterID(_characterName);

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("character", requestJSON);
        requestBody += GetMultipartFileData("file", characterSavePath);
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character";

            web::http::http_request request(web::http::methods::POST);

            request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            _backgroundTasks.run([endpoint, request]()
            {
                web::http::client::http_client httpClient((utility::string_t)endpoint);
                return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
                {
                    try
                    {
                        web::http::http_response response = responseTask.get();
                        if (response.status_code() == web::http::status_codes::OK)
                        {
                            Client::GetInstance().UpdateSeasonStanding();
                        }
                        else
                        {
                            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                        }
                    }
                    catch (std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
                    }
                });
            });
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
        }

        // TODO: Implement the other character data upload functions
        UploadCharacterQuestData(participantID);
        //UploadCharacterConversationData(participantID);
        //UploadCharacterFOWData(participantID);
    }

    return;
}

void ServerSync::UploadNewCharacterBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if (client.IsOfflineMode())
        return;

    uint32_t participantID = GetParticipantID(GameAPI::IsPlayerHardcore(_newPlayer));

    Character characterData;
    if (!characterData.ReadFromBuffer((uint8_t*)buffer, size))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
        return;
    }

    web::json::value characterJSON = characterData.ToJSON();
    web::json::value characterInfo = web::json::value::object();

    BuildCharacterInfo(characterInfo, characterJSON);

    web::json::value requestJSON;
    requestJSON[U("characterData")] = web::json::value::object();
    requestJSON[U("characterData")][U("characterInfo")] = characterInfo;
    requestJSON[U("characterData")][U("questInfo")] = web::json::value::object();
    requestJSON[U("seasonParticipantId")] = participantID;
    requestJSON[U("participantCharacterId")] = 0;

    std::string requestBody;
    requestBody += "\r\n";
    requestBody += GetMultipartJSONData("character", requestJSON);
    requestBody += GetMultipartBufferData("file", filename, (uint8_t*)buffer, size);
    requestBody += "--" + GetMultipartBoundary() + "--\r\n";

    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character";
        endpoint.AddParam("newPlayer", true);

        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        _backgroundTasks.run([endpoint, request]()
        {
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
            {
                try
                {
                    web::http::http_response response = responseTask.get();
                    if (response.status_code() != web::http::status_codes::OK)
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
                catch (std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
                }
            });
        });
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
    }
}

void ServerSync::UploadStashBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "shared-stash";

    web::json::value requestJSON;

    std::string requestBody;
    requestBody += "\r\n";
    requestBody += GetMultipartJSONData("sharedstash", requestJSON);
    requestBody += GetMultipartBufferData("file", filename, (uint8_t*)buffer, size);
    requestBody += "--" + GetMultipartBoundary() + "--\r\n";

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::POST);

    request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    // Cache the buffer data so that if it fails, we can send it again later
    // This needs to be cached because the transfer stash gets deleted before Shutdown/WorldUnload
    // Also, it's not safe to trust the save file because the user could replace it after the file has been saved
    _cachedStashBuffer = std::make_unique<FileWriter>((uint8_t*)buffer, size);

    _backgroundTasks.run([endpoint, request]()
    {
        web::http::client::http_client httpClient((utility::string_t)endpoint);
        return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
        {
            try
            {
                web::http::http_response response = responseTask.get();
                if (response.status_code() == web::http::status_codes::OK)
                {
                    ServerSync::GetInstance()._cachedStashBuffer.reset();
                }
                else
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
            }
        });
    });
}

void ServerSync::UploadCachedStashBuffer()
{
    if (_cachedStashBuffer)
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "shared-stash";

        web::json::value requestJSON;

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("sharedstash", requestJSON);
        requestBody += GetMultipartBufferData("file", GameAPI::GetTransferStashPath(), _cachedStashBuffer->GetBuffer(), _cachedStashBuffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        _backgroundTasks.run([endpoint, request]()
        {
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
            {
                try
                {
                    web::http::http_response response = responseTask.get();
                    if (response.status_code() == web::http::status_codes::OK)
                    {
                        ServerSync::GetInstance()._cachedStashBuffer.reset();
                    }
                    else
                    {
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                    }
                }
                catch (std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
                }
            });
        });
    }
}

void ServerSync::UploadCloudStash()
{
    Client& client = Client::GetInstance();
    if (client.IsPlayingSeasonOnline())
    {
        const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
        if (transferTabs.size() >= 6)
        {
            void* uploadTab = transferTabs[5];
            const std::map<uint32_t, EngineAPI::Rect>& items = GameAPI::GetItemsInTab(uploadTab);
            if (items.size() > 0)
            {
                uint32_t index = 0;
                web::json::value requestBody = web::json::value::array();
                for (const auto& pair : items)
                {
                    GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(EngineAPI::FindObjectByID(pair.first));

                    Item item = GameAPI::InfoToItem(itemInfo);
                    requestBody[index] = item.ToJSON();
                    requestBody[index].erase(U("unknown1"));
                    requestBody[index].erase(U("unknown2"));
                    index++;
                }

                GameAPI::RemoveAllItemsFromTab(uploadTab);

                URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "stash";
                endpoint.AddParam("branch", client.GetBranchName());

                web::http::client::http_client httpClient((utility::string_t)endpoint);
                web::http::http_request request(web::http::methods::POST);
                request.set_body(requestBody);

                std::string bearerToken = "Bearer " + client.GetAuthToken();
                request.headers().add(U("Authorization"), bearerToken.c_str());

                _backgroundTasks.run([endpoint, request]()
                {
                    web::http::client::http_client httpClient((utility::string_t)endpoint);
                    return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
                    {
                        try
                        {
                            web::http::http_response response = responseTask.get();
                            if (response.status_code() == web::http::status_codes::OK)
                            {
                                GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
                            }
                            else if (response.status_code() == web::http::status_codes::BadRequest)
                            {
                                GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
                            }
                            else
                            {
                                GameAPI::DisplayUINotification("tagGDLeagueStorageFailure");
                                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                            }
                        }
                        catch (std::exception& ex)
                        {
                            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
                        }
                    });
                });
            }
        }
    }
}

void ServerSync::PullTransferItems(const std::vector<std::shared_ptr<Item>>& items)
{
    Client& client = Client::GetInstance();
    if (client.IsPlayingSeason() && !client.IsOfflineMode())
    {
        web::json::value requestBody = web::json::value::array();
        for (uint32_t i = 0; i < items.size(); ++i)
        {
            requestBody[i] = items[i]->_itemID;
        }

        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "pull-items";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        _backgroundTasks.run([endpoint, request]()
        {
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            return httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
            {
                try
                {
                    web::http::http_response response = responseTask.get();
                    if (response.status_code() != web::http::status_codes::OK)
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
                catch (std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to pull items from transfer queue: %", ex.what());
                }
            });
        });
    }
}

void ServerSync::DownloadCharacterBuffer(const std::wstring& playerName, uint32_t participantID, void** data, size_t* size)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "file";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            std::vector<uint8_t> responseBody = response.extract_vector().get();
            *size = responseBody.size();
            *data = new uint8_t[*size];
            memcpy(*data, &responseBody[0], *size);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterFile(const std::wstring& playerName, uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "file";
        endpoint.AddParam("branch", client.GetBranchName());

        std::filesystem::path characterFilePath = GameAPI::GetPlayerSaveFile(playerName);
        if (std::filesystem::exists(characterFilePath))
            return;

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            std::vector<uint8_t> responseBody = response.extract_vector().get();
            FileWriter writer(&responseBody[0], responseBody.size());
            writer.WriteToFile(characterFilePath);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterQuestFile(const std::wstring& playerName, uint32_t participantID)
{
    try
    {
        for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
        {
            std::string difficultyName = GetGameDifficultyName(difficulty);

            std::filesystem::path characterPath = GameAPI::GetPlayerFolder(playerName);
            std::filesystem::path questFilePath = characterPath / "levels_world001.map" / difficultyName / "quests.gdd";
            if (std::filesystem::exists(questFilePath))
                return;

            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "questfile" / difficultyName;
            endpoint.AddParam("branch", client.GetBranchName());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                std::vector<uint8_t> responseBody = response.extract_vector().get();
                FileWriter writer(&responseBody[0], responseBody.size());
                writer.WriteToFile(questFilePath);
            }
            else
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character quest data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterList(uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "characters";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::array charactersArray = responseBody.as_array();
            for (size_t i = 0; i < charactersArray.size(); ++i)
            {
                std::wstring characterName = charactersArray[i].as_string();
                DownloadCharacterFile(characterName, participantID);
                DownloadCharacterQuestFile(characterName, participantID);
                // TODO: Download the other data (FOW/Conversation) from the server once it's been uploaded
            }
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character data: %", ex.what());
    }
}

void ServerSync::DownloadTransferItems(uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
        std::filesystem::path stashPath = GameAPI::GetTransferStashPath();

        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "transfer-queue";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::array itemsArray = response.extract_json().get().as_array();

            std::vector<std::shared_ptr<Item>> pullItemList;
            const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
            if ((transferTabs.size() >= 6) && (itemsArray.size() > 0))
            {
                void* downloadTab = transferTabs[4];
                for (auto it = itemsArray.begin(); it != itemsArray.end(); ++it)
                {
                    std::shared_ptr<Item> itemData(new Item(*it));
                    GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo(*itemData);
                    if (void* item = GameAPI::CreateItem(itemInfo))
                    {
                        if (GameAPI::AddItemToTransfer(EngineAPI::GetObjectID(item), 4, true))
                            pullItemList.push_back(itemData);

                        EngineAPI::DestroyObjectEx(item);
                    }
                    else
                    {
                        throw std::runtime_error("Failed to recreate stash item from item data");
                    }
                }
            }

            PullTransferItems(pullItemList);
            SetStashSynced(STASH_SYNC_TRANSFER);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download transfer items: %", ex.what());
    }
}

void ServerSync::DownloadStashData(uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "shared-stash" / "file";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            std::vector<uint8_t> responseBody = response.extract_vector().get();

            SharedStash stash;
            if (!stash.ReadFromBuffer(&responseBody[0], responseBody.size()))
            {
                throw std::runtime_error("Could not read stash data from server");
            }

            FileWriter writer(&responseBody[0], responseBody.size());
            writer.WriteToFile(GameAPI::GetTransferStashPath());

            GameAPI::SetNumberOfTransferTabs(stash.GetTabCount());

            const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
            for (size_t i = 0; i < transferTabs.size(); ++i)
            {
                GameAPI::RemoveAllItemsFromTab(transferTabs[i]);
                if (SharedStash::StashTab* tab = stash.GetStashTab(i))
                {
                    const ItemContainer::ItemList& itemList = tab->GetItemList();
                    for (auto it = itemList.begin(); it != itemList.end(); ++it)
                    {
                        uint32_t itemX = ((it->second >> 32) & 0xFFFFFFFF) * 32;
                        uint32_t itemY = (it->second & 0xFFFFFFFF) * 32;
                        EngineAPI::Vec2 position = EngineAPI::Vec2((float)itemX, (float)itemY);

                        GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo(*it->first);
                        if (void* item = GameAPI::CreateItem(itemInfo))
                        {
                            GameAPI::AddItemToTransfer(i, position, EngineAPI::GetObjectID(item), true);
                            EngineAPI::DestroyObjectEx(item);
                        }
                        else
                        {
                            throw std::runtime_error("Failed to recreate stash item from item data");
                        }
                    }
                }
            }

            SetStashSynced(STASH_SYNC_DOWNLOAD);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download stash data: %", ex.what());
    }
}

void ServerSync::SyncCharacterData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    Client& client = Client::GetInstance();
    if (std::filesystem::is_regular_file(filePath))
    {
        Character characterData;
        if (characterData.ReadFromFile(filePath))
        {
            std::filesystem::path parentPath = filePath.parent_path();
            std::wstring playerName = parentPath.filename().wstring().substr(1);

            web::json::value characterJSON = characterData.ToJSON();
            bool hardcore = (characterJSON[U("HeaderBlock")][U("Hardcore")].as_integer() != 0);
            uint32_t participantID = GetParticipantID(hardcore);

            FileMetadata clientMetadata(filePath);
            FileMetadata serverMetadata = GetServerCharacterMetadata(playerName, participantID);

            if (serverMetadata != clientMetadata)
            {
                DownloadCharacterBuffer(playerName, participantID, data, size);
                client.UpdateLeagueInfoText();
            }
        }
    }
}

void ServerSync::SyncStashData()
{
    IncrementStashLock();
    _backgroundTasks.run([&]()
    {
        Client& client = Client::GetInstance();
        uint32_t participantID = ServerSync::GetParticipantID(EngineAPI::IsHardcore());
        FileMetadata clientMetadata(GameAPI::GetTransferStashPath());
        FileMetadata serverMetadata = GetServerStashMetadata(participantID);

        if ((serverMetadata != clientMetadata) && (!IsStashSynced()))
        {
            DownloadStashData(participantID);
            DownloadTransferItems(participantID);
            client.UpdateLeagueInfoText();
        }
        else
        {
            SetStashSynced(STASH_SYNC_DOWNLOAD);
            DownloadTransferItems(participantID);
        }

        DecrementStashLock();
    });
}

void ServerSync::OnShutdownEvent()
{
    ServerSync& sync = ServerSync::GetInstance();
    sync.UploadCharacter();
    sync.UploadCachedStashBuffer();
    sync.WaitBackgroundComplete();
}

void ServerSync::OnDirectReadEvent(std::string filename, void** data, size_t* size, bool* override)
{
    std::filesystem::path filePath = filename;
    if (filePath.filename() == "player.gdc")
    {
        *size = 0;  // This should be set to 0 initially anyways, but do it again just to be safe

        ServerSync& sync = ServerSync::GetInstance();
        sync.SyncCharacterData(filePath, data, size);
        *override = (*size > 0);
    }
}

void ServerSync::OnDirectWriteEvent(std::string filename, void* data, size_t size)
{
    ServerSync& sync = ServerSync::GetInstance();
    std::filesystem::path filePath = filename;
    if (filePath == GameAPI::GetTransferStashPath())
    {
        sync.UploadStashBuffer(filename, data, size);
    }
}

void ServerSync::OnAddSaveJob(std::string filename, void* data, size_t size)
{
    ServerSync& sync = ServerSync::GetInstance();
    std::filesystem::path filePath(filename);
    if ((!std::filesystem::exists(filePath)) && (filePath.extension() == ".gdc") && (sync._newPlayer))
    {
        sync.UploadNewCharacterBuffer(filename, data, size);
        sync._newPlayer = nullptr;
    }
}

void ServerSync::OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded)
{
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode())
    {
        ServerSync& sync = ServerSync::GetInstance();
        if (mapName.substr(0, 16) == "levels/mainmenu/")
        {
            if (uint32_t softcoreID = sync.GetParticipantID(false))
                sync.DownloadCharacterList(softcoreID);

            if (uint32_t hardcoreID = sync.GetParticipantID(true))
                sync.DownloadCharacterList(hardcoreID);
        }
    }
}

void ServerSync::OnWorldPostLoadEvent(std::string mapName, bool unk1, bool modded)
{
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode())
    {
        ServerSync& sync = ServerSync::GetInstance();
        if (EngineAPI::IsMainCampaignOrCrucible())
        {
            sync.RegisterSeasonParticipant(EngineAPI::IsHardcore());
        }
    }
}

void ServerSync::OnWorldPreUnloadEvent()
{
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode())
    {
        ServerSync& sync = ServerSync::GetInstance();
        sync.UploadCharacter();
        sync.UploadCachedStashBuffer();
        sync.SetCurrentCharacterName({});
        sync.ResetStashSynced();
    }
}

void ServerSync::OnSetMainPlayerEvent(void* player)
{
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode())
    {
        ServerSync& sync = ServerSync::GetInstance();
        sync.SetCurrentCharacterName(GameAPI::GetPlayerName(player));
    }
}

void ServerSync::OnTransferPostLoadEvent()
{
    ServerSync& sync = ServerSync::GetInstance();
    sync.SyncStashData();
}

void ServerSync::OnTransferPreSaveEvent()
{
    Client& client = Client::GetInstance();
    ServerSync& sync = ServerSync::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        sync.UploadCloudStash();
    }
}

void ServerSync::OnCharacterPreSaveEvent(void* player)
{
    if (player)
    {
        Client& client = Client::GetInstance();
        ServerSync& sync = ServerSync::GetInstance();

        std::filesystem::path playerFile = GameAPI::GetPlayerSaveFile(player);
        if (GameAPI::IsPlayerInMainQuest(player))
        {
            // If the file doesn't exist, then it's most likely a new character so register the character with the server
            if ((!std::filesystem::exists(playerFile)) && (!GameAPI::HasPlayerBeenInGame(player)))
            {
                sync.RegisterSeasonParticipant(GameAPI::IsPlayerHardcore(player));
                sync._newPlayer = player;
            }
        }
    }
}

void ServerSync::OnDelayedCharacterUpload()
{
    ServerSync& sync = ServerSync::GetInstance();
    sync.UploadCharacter(0);
    sync.UploadCachedStashBuffer();
}

void ServerSync::OnCharacterPostSaveEvent(void* player)
{
    ThreadManager::CreatePeriodicThread("character_save", 1000, 0, 3000, &ServerSync::OnDelayedCharacterUpload);
}

void ServerSync::WaitBackgroundComplete()
{
    ServerSync& sync = ServerSync::GetInstance();
    sync._backgroundTasks.wait();
}