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
#include "Quest.h"
#include "Version.h"
#include "StringConvert.h"
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

std::string GetMultipartBufferData(const std::string& name, const std::filesystem::path& filePath, uint8_t* buffer, size_t size)
{
    std::ostringstream inputData;
    inputData.write((const char*)buffer, size);

    std::stringstream content;
    content << "--" << GetMultipartBoundary() << "\r\n";
    content << "Content-Disposition: form-data; name=\"" << name << "\"; filename=\"" << filePath.filename().string() << "\"\r\n"
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
    Client& client = Client::GetInstance();
    if (!client.IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,            &ServerSync::OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,              &ServerSync::OnShutdownEvent);
        EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_READ,      &ServerSync::OnDirectReadEvent);
        EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_WRITE,     &ServerSync::OnDirectWriteEvent);
        EventManager::Subscribe(GDCL_EVENT_ADD_SAVE_JOB,          &ServerSync::OnAddSaveJobEvent);
        EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,        &ServerSync::OnWorldPreLoadEvent);
        //EventManager::Subscribe(GDCL_EVENT_WORLD_POST_LOAD,       &ServerSync::OnWorldPostLoadEvent);
        EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD,      &ServerSync::OnWorldPreUnloadEvent);
        EventManager::Subscribe(GDCL_EVENT_SET_MAIN_PLAYER,       &ServerSync::OnSetMainPlayerEvent);
        EventManager::Subscribe(GDCL_EVENT_TRANSFER_POST_LOAD,    &ServerSync::OnTransferPostLoadEvent);
        EventManager::Subscribe(GDCL_EVENT_TRANSFER_PRE_SAVE,     &ServerSync::OnTransferPreSaveEvent);
        EventManager::Subscribe(GDCL_EVENT_TRANSFER_POST_SAVE,    &ServerSync::OnTransferPostSaveEvent);
        //EventManager::Subscribe(GDCL_EVENT_CHARACTER_PRE_SAVE,    &ServerSync::OnCharacterPreSaveEvent);
        EventManager::Subscribe(GDCL_EVENT_CHARACTER_POST_SAVE,   &ServerSync::OnCharacterPostSaveEvent);
        EventManager::Subscribe(GDCL_EVENT_DELETE_FILE,           &ServerSync::OnDeleteFileEvent);

        /*if (Connection* connection = client.GetConnection())
        {
            connection->Register("AddSeasonParticipant",          &ServerSync::OnAddParticipant);
            connection->Register("GetParticipantCharacter",       &ServerSync::OnGetCharacterInfo);
            connection->Register("GetParticipantCharacters",      &ServerSync::OnGetCharacterList);
            connection->Register("GetParticipantSharedStash",     &ServerSync::OnGetStashInfo);
            connection->Register("ConnectionStatus",              &ServerSync::OnConnectionStatus);
        }*/
    }
}

ServerSync& ServerSync::GetInstance()
{
    static ServerSync instance;
    return instance;
}

uint32_t ServerSync::GetParticipantID(bool hardcore)
{
    if (_participantIDCache.count(hardcore) > 0)
        return _participantIDCache[hardcore];

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
                    {
                        _participantIDCache[hardcore] = participantID;
                        return participantID;
                    }
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

uint32_t ServerSync::GetCharacterID(const std::wstring& playerName)
{
    Client& client = Client::GetInstance();
    uint32_t participantID = client.GetCurrentParticipantID();

    if (_characterIDCache.count(playerName) > 0)
        return _characterIDCache[playerName];

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
                _characterIDCache[playerName] = (uint32_t)characterID.as_integer();
                return _characterIDCache[playerName];
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
    }
    return 0;
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
    return result;
}

int32_t ServerSync::GetServerStashCapacity(uint32_t participantID)
{
    int32_t result = 0;
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
                return responseBody[U("cloudStashCapacity")].as_integer();
            }
            case web::http::status_codes::NoContent:
            {
                return -1;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve shared stash capacity: %", ex.what());
        return -1;
    }
}

void ServerSync::RegisterSeasonParticipant(bool hardcore, bool async)
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

void ServerSync::UploadNewCharacterBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if (client.IsOfflineMode())
        return;

    Character characterData;
    if (!characterData.ReadFromBuffer((uint8_t*)buffer, size))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
        return;
    }

    bool hardcore = characterData._headerBlock._charIsHardcore;
    uint32_t participantID = GetParticipantID(hardcore);
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

    URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character";
    endpoint.AddParam("branch", client.GetBranchName());
    endpoint.AddParam("newPlayer", true);

    web::http::http_request request(web::http::methods::POST);

    request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    httpClient.request(request).then([](web::http::http_response response)
    {
        if (response.status_code() == web::http::status_codes::OK)
            return true;
        else
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
    })
    .then([](concurrency::task<bool> task)
    {
        try
        {
            task.get();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
        }
    });
}

void ServerSync::UploadStashBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "shared-stash";

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
        _cachedStashBuffer = std::make_shared<StashBuffer>((uint8_t*)buffer, size, EngineAPI::IsHardcore());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    ServerSync::GetInstance()._cachedStashBuffer.reset();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadFormulasBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "formulasfile";

        web::json::value requestJSON;

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("formulas", requestJSON);
        requestBody += GetMultipartBufferData("file", filename, (uint8_t*)buffer, size);
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                task.get();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload formulas file: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadTransmutesBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "transmutesfile";

        web::json::value requestJSON;

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("transmutes", requestJSON);
        requestBody += GetMultipartBufferData("file", filename, (uint8_t*)buffer, size);
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                task.get();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload transmutes file: %", ex.what());
            }
        });
    }
}

void ServerSync::CacheCharacterBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        _cachedCharacterBuffer = std::make_shared<CharacterBuffer>((uint8_t*)buffer, size);
        _cachedCharacterBuffer->_tagData = web::json::value::object();

        for (auto pair : difficultyTagLookup)
            _cachedCharacterBuffer->_tagData[pair.second] = web::json::value::array();

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
                        _cachedCharacterBuffer->_tagData[difficultyTagLookup.at(difficulty)][index++] = JSONString(tokenString);
                }
            }
        }
    }
}

void ServerSync::CacheQuestBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (EngineAPI::IsMainCampaign()) && (!EngineAPI::IsMultiplayer()))
    {
        if (void* mainPlayer = GameAPI::GetMainPlayer())
        {
            GameAPI::Difficulty difficulty = GameAPI::GetGameDifficulty();
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);

            _cachedQuestBuffer = std::make_shared<PlayerDataBuffer>((uint8_t*)buffer, size, difficulty, playerName);
        }
    }
}

void ServerSync::CacheConversationsBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (EngineAPI::IsMainCampaign()) && (!EngineAPI::IsMultiplayer()))
    {
        if (void* mainPlayer = GameAPI::GetMainPlayer())
        {
            GameAPI::Difficulty difficulty = GameAPI::GetGameDifficulty();
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);

            _cachedConversationsBuffer = std::make_shared<PlayerDataBuffer>((uint8_t*)buffer, size, difficulty, playerName);
        }
    }
}

void ServerSync::CacheFOWBuffer(const std::string& filename, void* buffer, size_t size)
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (EngineAPI::IsMainCampaign()) && (!EngineAPI::IsMultiplayer()))
    {
        if (void* mainPlayer = GameAPI::GetMainPlayer())
        {
            GameAPI::Difficulty difficulty = GameAPI::GetGameDifficulty();
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);

            _cachedFOWBuffer = std::make_shared<PlayerDataBuffer>((uint8_t*)buffer, size, difficulty, playerName);
        }
    }
}

void ServerSync::UploadCachedCharacterBuffer()
{
    std::shared_ptr<CharacterBuffer> characterBuffer = _cachedCharacterBuffer;
    if (characterBuffer)
    {
        Client& client = Client::GetInstance();
        uint32_t participantID = client.GetCurrentParticipantID();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character";
        endpoint.AddParam("branch", client.GetBranchName());

        Character characterData;
        if (!characterData.ReadFromBuffer(characterBuffer->GetBuffer(), characterBuffer->GetBufferSize()))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load cached character data.");
            return;
        }

        web::json::value characterJSON = characterData.ToJSON();
        web::json::value characterInfo = web::json::value::object();

        BuildCharacterInfo(characterInfo, characterJSON);

        std::wstring characterName = characterJSON[U("HeaderBlock")][U("Name")].as_string();

        web::json::value requestJSON;
        requestJSON[U("characterData")] = web::json::value::object();
        requestJSON[U("characterData")][U("characterInfo")] = characterInfo;
        requestJSON[U("characterData")][U("questInfo")] = characterBuffer->_tagData;
        requestJSON[U("seasonParticipantId")] = participantID;
        requestJSON[U("participantCharacterId")] = GetCharacterID(characterName);

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("character", requestJSON);
        requestBody += GetMultipartBufferData("file", GameAPI::GetPlayerSaveFile(characterName), characterBuffer->GetBuffer(), characterBuffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());


        web::http::client::http_client httpClient((utility::string_t)endpoint);
        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                {
                    ServerSync::GetInstance()._cachedCharacterBuffer.reset();
                    Client::GetInstance().UpdateSeasonStanding();
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadCachedQuestBuffer()
{
    std::shared_ptr<PlayerDataBuffer> buffer = _cachedQuestBuffer;
    if (buffer)
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "character" / buffer->_playerName / "questfile" / GameAPI::GetGameDifficultyName(buffer->_difficulty);
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartBufferData("file", "quests.gdd", buffer->GetBuffer(), buffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    ServerSync::GetInstance()._cachedQuestBuffer.reset();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload quest data: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadCachedConversationsBuffer()
{
    std::shared_ptr<PlayerDataBuffer> buffer = _cachedConversationsBuffer;
    if (buffer)
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "character" / buffer->_playerName / "conversationsfile" / GameAPI::GetGameDifficultyName(buffer->_difficulty);
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartBufferData("file", "conversations.gdd", buffer->GetBuffer(), buffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    ServerSync::GetInstance()._cachedConversationsBuffer.reset();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload conversations data: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadCachedFOWBuffer()
{
    std::shared_ptr<PlayerDataBuffer> buffer = _cachedFOWBuffer;
    if (buffer)
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "character" / buffer->_playerName / "mapfowfile" / GameAPI::GetGameDifficultyName(buffer->_difficulty);
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartBufferData("file", "map.fow", buffer->GetBuffer(), buffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    ServerSync::GetInstance()._cachedFOWBuffer.reset();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload FOW data: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadCachedStashBuffer()
{
    std::shared_ptr<StashBuffer> buffer = _cachedStashBuffer;
    if (buffer)
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "shared-stash";

        web::json::value requestJSON;

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("sharedstash", requestJSON);
        requestBody += GetMultipartBufferData("file", (buffer->_hardcore ? "transfer.gsh" : "transfer.gst"), buffer->GetBuffer(), buffer->GetBufferSize());
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    ServerSync::GetInstance()._cachedStashBuffer.reset();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
            }
        });
    }
}

void ServerSync::UploadCachedBuffers()
{
    UploadCachedCharacterBuffer();
    UploadCachedQuestBuffer();
    UploadCachedConversationsBuffer();
    UploadCachedFOWBuffer();
    UploadCachedStashBuffer();
}

void ServerSync::SaveTagsFile()
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "tagsfile";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::POST);

    std::string filename = EngineAPI::IsHardcore() ? "tags.gsh" : "tags.gst";
    std::filesystem::path filePath = GameAPI::GetUserSaveFolder() / filename;

    void* mainPlayer = GameAPI::GetMainPlayer();
    const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, GameAPI::GetGameDifficulty());

    uint32_t count = 0;
    std::string buffer;
    for (GameAPI::TriggerToken token : tokens)
    {
        std::string tokenString = token;
        if (tokenString.rfind("GDIP_", 0) == 0)
        {
            uint32_t tokenSize = tokenString.size();
            buffer.append((const char*)&tokenSize, 4);
            buffer += tokenString;
            count++;
        }
    }

    size_t fileSize = 8 + buffer.size();
    FileWriter writer(fileSize);

    writer.BufferInt32(count);
    writer.BufferString(buffer);
    writer.WriteToFile(filePath);

    std::string requestBody;
    requestBody += "\r\n";
    requestBody += GetMultipartBufferData("file", filePath.filename(), writer.GetBuffer(), writer.GetBufferSize());
    requestBody += "--" + GetMultipartBoundary() + "--\r\n";

    request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    httpClient.request(request).then([](web::http::http_response response)
    {
        if (response.status_code() == web::http::status_codes::OK)
            return true;
        else
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
    })
    .then([](concurrency::task<bool> task)
    {
        try
        {
            task.get();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared tags file: %", ex.what());
        }
    });
}

void RefundCloudStashItems(const std::vector<GameAPI::ItemReplicaInfo> items)
{
    void* mainPlayer = GameAPI::GetMainPlayer();
    uint32_t playerID = EngineAPI::GetObjectID(GameAPI::GetMainPlayer());

    for (const auto& itemInfo : items)
    {
        if (void* item = GameAPI::CreateItem(itemInfo))
        {
            GameAPI::SetItemVisiblePlayer(item, playerID);
            GameAPI::AddItemToTransfer(EngineAPI::GetObjectID(item), 5, true);
            EngineAPI::DestroyObjectEx(item);
        }
        else
        {
            throw std::runtime_error("Failed to recreate stash item from item data");
        }
    }
}

void ServerSync::UploadCloudStash()
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
        if (transferTabs.size() >= 6)
        {
            void* uploadTab = transferTabs[5];
            const std::map<uint32_t, EngineAPI::Rect>& items = GameAPI::GetItemsInTab(uploadTab);

            if (items.size() > 0)
            {
                int32_t capacity = GetServerStashCapacity(client.GetCurrentParticipantID());
                if ((items.size() > capacity) && (capacity >= 0))
                {
                    GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
                    return;
                }

                IncrementStashLock();

                uint32_t index = 0;
                std::vector<GameAPI::ItemReplicaInfo> cloudStashItems;
                web::json::value requestBody = web::json::value::array();
                for (const auto& pair : items)
                {
                    void* item = EngineAPI::FindObjectByID(pair.first);
                    if (item)
                    {
                        GameAPI::ItemReplicaInfo itemInfo;
                        GameAPI::GetItemReplicaInfo(item, itemInfo);
                        cloudStashItems.push_back(itemInfo);

                        Item item = GameAPI::InfoToItem(itemInfo);
                        requestBody[index] = item.ToJSON();
                        requestBody[index].erase(U("unknown1"));
                        requestBody[index].erase(U("unknown2"));
                        index++;
                    }
                }

                GameAPI::RemoveAllItemsFromTab(uploadTab);

                Client& client = Client::GetInstance();
                URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "stash";
                endpoint.AddParam("branch", client.GetBranchName());

                web::http::client::http_client httpClient((utility::string_t)endpoint);
                web::http::http_request request(web::http::methods::POST);
                request.set_body(requestBody);

                std::string bearerToken = "Bearer " + client.GetAuthToken();
                request.headers().add(U("Authorization"), bearerToken.c_str());

                httpClient.request(request).then([](web::http::http_response response)
                {
                    return response.status_code();
                })
                .then([cloudStashItems](concurrency::task<web::http::status_code> task)
                {
                    try
                    {
                        web::http::status_code statusCode = task.get();
                        if (statusCode == web::http::status_codes::OK)
                        {
                            const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
                            GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
                        }
                        else if (statusCode == web::http::status_codes::BadRequest)
                        {
                            RefundCloudStashItems(cloudStashItems);
                            GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
                        }
                        else
                        {
                            RefundCloudStashItems(cloudStashItems);
                            GameAPI::DisplayUINotification("tagGDLeagueStorageFailure");
                            throw std::runtime_error("Server responded with status code " + std::to_string(statusCode));
                        }
                    }
                    catch (std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload items to cloud stash: %", ex.what());
                    }
                    GetInstance().DecrementStashLock();
                });
            }
        }
    }
}

void ServerSync::LoadQuestStatesForPlayer(void* player)
{
    if (player)
    {
        try
        {
            uint32_t participantID = GetParticipantID(GameAPI::IsPlayerHardcore(player));
            std::string difficultyName = GetGameDifficultyName(GameAPI::GetGameDifficulty());
            std::wstring playerName = GameAPI::GetPlayerName(player);

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

                Quest questData;
                if (!questData.ReadFromBuffer(&responseBody[0], responseBody.size()))
                {
                    throw std::runtime_error("Could not read quest data from server");
                }

                GameAPI::ClearPlayerTokens(player);
                for (const auto& token : questData._tokensBlock._questTokens)
                {
                    // Skip loading these, as they will be loaded from the tags file
                    if (token.rfind("GDIP_", 0) == 0)
                        continue;

                    GameAPI::BestowTokenNow(player, token);
                }

                for (const auto& questData : questData._dataBlock._questData)
                {
                    void* quest = GameAPI::GetQuestByID(questData._id1);
                    if (!quest)
                        throw std::runtime_error("Failed to load quest data with ID " + std::to_string(questData._id1));

                    for (const auto& taskData : questData._tasks)
                    {
                        void* task = GameAPI::GetQuestTaskByID(quest, taskData._id1);
                        if (!task)
                            throw std::runtime_error("Failed to load task " + std::to_string(taskData._id1) + " for quest with ID " + std::to_string(questData._id1));

                        GameAPI::SetQuestTaskState(task, taskData._state);
                        GameAPI::SetQuestTaskInProgress(task, taskData._isInProgress);
                    }
                }
            }
            else if (response.status_code() != web::http::status_codes::NoContent)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character quest states: %", ex.what());
        }
    }
}

void ServerSync::LoadTagsFileForPlayer(void* player)
{
    if (player)
    {
        try
        {
            uint32_t participantID = GetParticipantID(GameAPI::IsPlayerHardcore(player));
            std::string difficultyName = GetGameDifficultyName(GameAPI::GetGameDifficulty());
            std::wstring playerName = GameAPI::GetPlayerName(player);

            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "tagsfile";
            endpoint.AddParam("branch", client.GetBranchName());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                std::vector<uint8_t> responseBody = response.extract_vector().get();

                FileReader reader(&responseBody[0], responseBody.size());

                uint32_t count = reader.ReadInt32();
                reader.ReadInt32();     // This is the size of the string contents, but not really needed here

                for (size_t i = 0; i < count; ++i)
                {
                    std::string tokenString = reader.ReadString();
                    GameAPI::BestowTokenNow(player, tokenString);
                }
            }
            else if (response.status_code() != web::http::status_codes::NoContent)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load shared tags file: %", ex.what());
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
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "pull-items";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                task.get();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to pull items from transfer queue: %", ex.what());
            }
        });
    }
}

void ServerSync::DownloadCharacterBuffer(std::wstring playerName, uint32_t participantID, void** data, size_t* size)
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
        else if (response.status_code() != web::http::status_codes::NoContent)
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterFile(std::wstring playerName, uint32_t participantID, bool overwrite)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "file";
        endpoint.AddParam("branch", client.GetBranchName());

        std::filesystem::path characterFilePath = GameAPI::GetPlayerSaveFile(playerName);
        if ((!overwrite) && (std::filesystem::exists(characterFilePath)))
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
        else if (response.status_code() != web::http::status_codes::NoContent)
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterQuestData(const std::wstring& playerName, uint32_t participantID, bool overwrite)
{
    try
    {
        for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
        {
            std::string difficultyName = GetGameDifficultyName(difficulty);

            std::filesystem::path characterPath = GameAPI::GetPlayerFolder(playerName);
            std::filesystem::path questFilePath = characterPath / "levels_world001.map" / difficultyName / "quests.gdd";
            if ((!overwrite) && (std::filesystem::exists(questFilePath)))
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
            else if (response.status_code() != web::http::status_codes::NoContent)
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

void ServerSync::DownloadCharacterConversationsData(const std::wstring& playerName, uint32_t participantID, bool overwrite)
{
    try
    {
        for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
        {
            std::string difficultyName = GetGameDifficultyName(difficulty);

            std::filesystem::path characterPath = GameAPI::GetPlayerFolder(playerName);
            std::filesystem::path questFilePath = characterPath / "levels_world001.map" / difficultyName / "conversations.gdd";
            if ((!overwrite) && (std::filesystem::exists(questFilePath)))
                return;

            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "conversationsfile" / difficultyName;
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
            else if (response.status_code() != web::http::status_codes::NoContent)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character conversations data: %", ex.what());
    }
}

void ServerSync::DownloadCharacterFOWData(const std::wstring& playerName, uint32_t participantID, bool overwrite)
{
    try
    {
        for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
        {
            std::string difficultyName = GetGameDifficultyName(difficulty);

            std::filesystem::path characterPath = GameAPI::GetPlayerFolder(playerName);
            std::filesystem::path questFilePath = characterPath / "levels_world001.map" / difficultyName / "map.fow";
            if ((!overwrite) && (std::filesystem::exists(questFilePath)))
                return;

            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "mapfowfile" / difficultyName;
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
            else if (response.status_code() != web::http::status_codes::NoContent)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download character FOW data: %", ex.what());
    }
}

void ServerSync::CleanupSaveFolder(const std::unordered_set<std::wstring>& characterList)
{
    std::filesystem::path mainPath = GameAPI::GetUserSaveFolder() / "main";
    if (std::filesystem::is_directory(mainPath))
    {
        for (const auto& it : std::filesystem::directory_iterator(mainPath))
        {
            const std::filesystem::path& filePath = it.path();
            if (std::filesystem::is_directory(filePath))
            {
                std::wstring playerName = filePath.filename().wstring().substr(1);
                if (characterList.count(playerName) > 0)
                    continue;
            }
            std::filesystem::remove_all(filePath);
        }
    }
}

void ServerSync::DownloadCharacterList(std::vector<pplx::task<void>>& downloadTasks, uint32_t participantID, std::unordered_set<std::wstring>& characterList)
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
                std::wstring playerName = charactersArray[i].as_string();

                auto task = pplx::create_task([=]()
                {
                    DownloadCharacterFile(playerName, participantID);
                    DownloadCharacterQuestData(playerName, participantID);
                    DownloadCharacterConversationsData(playerName, participantID);
                    DownloadCharacterFOWData(playerName, participantID);
                })
                .then([](pplx::task<void> task)
                {
                    try
                    {
                        task.get();
                    }
                    catch (std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to download character data: %", ex.what());
                    }
                });

                downloadTasks.emplace_back(task);
                characterList.insert(playerName);
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

void ServerSync::DownloadFormulasFile(std::vector<pplx::task<void>>& downloadTasks, uint32_t participantID, bool hardcore)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "formulasfile";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        auto task = httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return response.extract_vector();
            else if (response.status_code() == web::http::status_codes::NoContent)
                return pplx::create_task([]() -> std::vector<uint8_t> { return {}; });
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([hardcore](concurrency::task<std::vector<uint8_t>> task)
        {
            try
            {
                std::vector<uint8_t> responseBody = task.get();
                if (!responseBody.empty())
                {
                    FileWriter writer(&responseBody[0], responseBody.size());
                    writer.WriteToFile(GameAPI::GetFormulasPath(hardcore));
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download formulas file: %", ex.what());
            }
        });

        downloadTasks.emplace_back(task);
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download formulas file: %", ex.what());
    }
}

void ServerSync::DownloadTransmutesFile(std::vector<pplx::task<void>>& downloadTasks, uint32_t participantID, bool hardcore)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "transmutesfile";
        endpoint.AddParam("branch", client.GetBranchName());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        auto task = httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return response.extract_vector();
            else if (response.status_code() == web::http::status_codes::NoContent)
                return pplx::create_task([]() -> std::vector<uint8_t> { return {}; });
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([hardcore](concurrency::task<std::vector<uint8_t>> task)
        {
            try
            {
                std::vector<uint8_t> responseBody = task.get();
                if (!responseBody.empty())
                {
                    FileWriter writer(&responseBody[0], responseBody.size());
                    writer.WriteToFile(GameAPI::GetTransmutesPath(hardcore));
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download transmutes file: %", ex.what());
            }
        });

        downloadTasks.emplace_back(task);
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download formulas file: %", ex.what());
    }
}

void ServerSync::DownloadTransferItems(uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
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

            uint32_t playerID = EngineAPI::GetObjectID(GameAPI::GetMainPlayer());
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
                        GameAPI::SetItemVisiblePlayer(item, playerID);
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

            GameAPI::SaveTransferStash();
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
        endpoint.AddParam("branch", client.GetBranchName());

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
            writer.WriteToFile(GameAPI::GetTransferStashPath(EngineAPI::IsHardcore()));

            GameAPI::SetNumberOfTransferTabs(stash.GetTabCount());

            uint32_t playerID = EngineAPI::GetObjectID(GameAPI::GetMainPlayer());
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
                            GameAPI::SetItemVisiblePlayer(item, playerID);
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
    Character character;
    if (character.ReadFromFile(filePath, true))
    {
        uint32_t participantID = GetParticipantID(character._headerBlock._charIsHardcore);
        std::wstring playerName = filePath.parent_path().filename().wstring().substr(1);
        FileMetadata serverMetadata = GetServerCharacterMetadata(playerName, participantID);
        FileMetadata clientMetadata(filePath);

        if (serverMetadata != clientMetadata)
        {
            DownloadCharacterBuffer(playerName, participantID, data, size);
            DownloadCharacterQuestData(playerName, participantID, true);
            DownloadCharacterConversationsData(playerName, participantID);
            DownloadCharacterFOWData(playerName, participantID);
        }
    }
}

void ServerSync::SyncStashData()
{
    IncrementStashLock();

    pplx::create_task([=]()
    {
        Client& client = Client::GetInstance();
        uint32_t participantID = GetParticipantID(EngineAPI::IsHardcore());
        FileMetadata clientMetadata(GameAPI::GetTransferStashPath(EngineAPI::IsHardcore()));
        FileMetadata serverMetadata = GetServerStashMetadata(participantID);

        if ((serverMetadata != clientMetadata) && (!IsStashSynced()))
        {
            DownloadStashData(participantID);
            DownloadTransferItems(participantID);
        }
        else
        {
            SetStashSynced(STASH_SYNC_DOWNLOAD);
            DownloadTransferItems(participantID);
        }
    })
    .then([=](pplx::task<void> task)
    {
        try
        {
            task.get();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to sync stash data: %", ex.what());
        }
        DecrementStashLock();
    });
}

void ServerSync::OnInitializeEvent()
{
    // Connect the client at game start; we can't do this when the DLL is loaded due to networking code
    //Client& client = Client::GetInstance();
    //if (Connection* connection = client.GetConnection())
    //    connection->Connect();
}

void ServerSync::OnShutdownEvent()
{
    GetInstance().UploadCachedBuffers();
    //GetInstance().WaitBackgroundComplete();

    //Client& client = Client::GetInstance();
    //if (Connection* connection = client.GetConnection())
    //    connection->Disconnect();
}

void ServerSync::OnDirectReadEvent(std::string filename, void** data, size_t* size)
{
    std::filesystem::path filePath = filename;
    if ((filePath.filename() == "player.gdc") && (std::filesystem::is_regular_file(filePath)))
    {
        GetInstance().SyncCharacterData(filePath, data, size);
    }
}

void ServerSync::OnDirectWriteEvent(std::string filename, void* data, size_t size)
{
    std::filesystem::path filePath = filename;
    if (filePath == GameAPI::GetTransferStashPath(EngineAPI::IsHardcore()))
    {
        GetInstance().UploadStashBuffer(filename, data, size);
    }
    else if (filePath == GameAPI::GetFormulasPath(EngineAPI::IsHardcore()))
    {
        GetInstance().UploadFormulasBuffer(filename, data, size);
    }
    else if (filePath == GameAPI::GetTransmutesPath(EngineAPI::IsHardcore()))
    {
        GetInstance().UploadTransmutesBuffer(filename, data, size);
    }
}

void ServerSync::OnAddSaveJobEvent(std::string filename, void* data, size_t size)
{
    std::filesystem::path filePath(filename);
    if (filePath.extension() == ".gdc")
    {
        if (!std::filesystem::exists(filePath))
        {
            GetInstance().UploadNewCharacterBuffer(filename, data, size);
        }
        else
        {
            // Cache the character data instead of uploading immediately so that we don't overwhelm the server with requests
            GetInstance().CacheCharacterBuffer(filename, data, size);
        }
    }
    // Only save these files in the main campaign to prevent Crucible from overwriting them
    else if (EngineAPI::IsMainCampaign())
    {
        if (filePath.filename() == "quests.gdd")
        {
            GetInstance().CacheQuestBuffer(filename, data, size);

            // Only save tags file if the character already exists, so that we don't overwrite all of them on a new character
            if (std::filesystem::exists(filePath))
                GetInstance().SaveTagsFile();
        }
        else if (filePath.filename() == "conversations.gdd")
        {
            GetInstance().CacheConversationsBuffer(filename, data, size);
        }
        else if (filePath.filename() == "map.fow")
        {
            GetInstance().CacheFOWBuffer(filename, data, size);
        }
    }
}

void ServerSync::OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded)
{
    if (mapName.substr(0, 16) == "levels/mainmenu/")
    {
        std::vector<pplx::task<void>> downloadTasks;
        std::unordered_set<std::wstring> characterList;
        if (uint32_t softcoreID = GetInstance().GetParticipantID(false))
        {
            GetInstance().DownloadCharacterList(downloadTasks, softcoreID, characterList);
            GetInstance().DownloadFormulasFile(downloadTasks, softcoreID, false);
            GetInstance().DownloadTransmutesFile(downloadTasks, softcoreID, false);
        }

        if (uint32_t hardcoreID = GetInstance().GetParticipantID(true))
        {
            GetInstance().DownloadCharacterList(downloadTasks, hardcoreID, characterList);
            GetInstance().DownloadFormulasFile(downloadTasks, hardcoreID, true);
            GetInstance().DownloadTransmutesFile(downloadTasks, hardcoreID, true);
        }
        pplx::when_all(downloadTasks.begin(), downloadTasks.end()).wait();
        GetInstance().CleanupSaveFolder(characterList);
    }
}

void ServerSync::OnWorldPostLoadEvent(std::string mapName, bool unk1, bool modded)
{
    //if (EngineAPI::IsMainCampaignOrCrucible())
    //    ServerSync::GetInstance().RegisterSeasonParticipant(EngineAPI::IsHardcore());
}

void ServerSync::OnWorldPreUnloadEvent()
{
    ServerSync::GetInstance().UploadCachedBuffers();
    ServerSync::GetInstance().ResetStashSynced();
}

void ServerSync::OnSetMainPlayerEvent(void* player)
{
    GetInstance().LoadQuestStatesForPlayer(player);
    GetInstance().LoadTagsFileForPlayer(player);
}

void ServerSync::OnTransferPostLoadEvent()
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        GetInstance().SyncStashData();
    }
}

void ServerSync::OnTransferPreSaveEvent()
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        ServerSync::GetInstance().UploadCloudStash();
    }
}

void ServerSync::OnTransferPostSaveEvent()
{
    Client& client = Client::GetInstance();
    if ((client.IsPlayingSeasonOnline()) && (!EngineAPI::IsMultiplayer()))
    {
        // Force a character save, which will trigger a character upload to the server
        // This ensures that players can't dupe items by going offline after placing them in the stash
        GameAPI::SaveGame();
    }
}

void ServerSync::OnCharacterPreSaveEvent(void* player)
{
    /*if (player)
    {
        std::filesystem::path playerFile = GameAPI::GetPlayerSaveFile(player);
        if (GameAPI::IsPlayerInMainQuest(player))
        {
            // If the file doesn't exist, then it's most likely a new character so register the character with the server
            if ((!std::filesystem::exists(playerFile)) && (!GameAPI::HasPlayerBeenInGame(player)))
            {
                GetInstance().RegisterSeasonParticipant(GameAPI::IsPlayerHardcore(player));
            }
        }
    }*/
}

void ServerSync::OnCharacterPostSaveEvent(void* player)
{
    ThreadManager::CreatePeriodicThread("character_save", 1000, 0, 5000, &ServerSync::OnDelayedCharacterUpload);
}

void ServerSync::OnDeleteFileEvent(const char* filename)
{
    std::filesystem::path filePath(filename);
    if (filePath.filename() == "player.gdc")
    {
        std::filesystem::path parentPath = filePath.parent_path();
        std::wstring playerName = parentPath.filename().wstring().substr(1);

        // Check to make sure it's a season character and not a custom game character before deleting
        if (parentPath.parent_path().filename() == "main")
        {
            Character characterData;
            if (!characterData.ReadFromFile(filename, true))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
                return;
            }

            Client& client = Client::GetInstance();
            uint32_t participantID = GetInstance().GetParticipantID(characterData._headerBlock._charIsHardcore);
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName;
            endpoint.AddParam("branch", client.GetBranchName());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::DEL);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            httpClient.request(request).then([](Concurrency::task<web::http::http_response> responseTask)
            {
                try
                {
                    web::http::http_response response = responseTask.get();
                    if (response.status_code() != web::http::status_codes::OK)
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
                catch (std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to delete character data: %", ex.what());
                }
            });
        }
    }
}

void ServerSync::OnDelayedCharacterUpload()
{
    GetInstance().UploadCachedBuffers();
}

void ServerSync::OnAddParticipant(const signalr::value& value)
{
    const signalr::value& result = value.as_array()[0];
    if (result.is_string())
    {
        Client& client = Client::GetInstance();
        web::json::value resultJSON = web::json::value::parse(result.as_string());
        if (!resultJSON.is_null())
        {
            client.SetParticipantID(resultJSON[U("SeasonParticipantId")].as_integer());
        }
    }
}

void ServerSync::OnGetCharacterInfo(const signalr::value& value, const std::vector<void*> args)
{
    const signalr::value& result = value.as_array()[0];
    if (result.is_string())
    {
        auto characterID = static_cast<uint32_t*>(args[0]);
        auto checksum = static_cast<std::string*>(args[1]);

        web::json::value resultJSON = web::json::value::parse(result.as_string());
        if (!resultJSON.is_null())
        {
            if (characterID != nullptr)
                *characterID = resultJSON[U("ParticipantCharacterId")].as_integer();

            if (checksum != nullptr)
                *checksum = JSONString(resultJSON[U("LastChecksum")].serialize());
        }
    }
}

void ServerSync::OnGetCharacterList(const signalr::value& value, const std::vector<void*> args)
{
    const signalr::value& result = value.as_array()[0];
    if (result.is_array())
    {
        auto characterList = static_cast<std::unordered_set<std::wstring>*>(args[0]);
        characterList->clear();

        const std::vector<signalr::value>& charactersArray = result.as_array();
        for (size_t i = 0; i < charactersArray.size(); ++i)
        {
            std::wstring playerName = CharToWide(charactersArray[i].as_string());
            playerName = std::wstring(playerName.begin() + 1, playerName.end() - 1);
            characterList->insert(playerName);
        }
    }
}

void ServerSync::OnGetStashInfo(const signalr::value& value, const std::vector<void*> args)
{
    const signalr::value& result = value.as_array()[0];
    if (result.is_string())
    {
        auto checksum = static_cast<std::string*>(args[0]);
        auto capacity = static_cast<int32_t*>(args[1]);

        web::json::value resultJSON = web::json::value::parse(result.as_string());
        if (!resultJSON.is_null())
        {
            if (checksum != nullptr)
                *checksum = JSONString(resultJSON[U("LastChecksum")].serialize());

            if (capacity != nullptr)
                *capacity = resultJSON[U("CloudStashCapacity")].as_integer();
        }
    }
}

void ServerSync::OnConnectionStatus(const signalr::value& value, const std::vector<void*> args)
{
    // No need to do anything here; calling the method is enough for checking connection status
}