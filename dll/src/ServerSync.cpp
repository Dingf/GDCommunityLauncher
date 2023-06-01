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

std::string GetMultipartJSONData(const std::string& name, const web::json::value& data)
{
    std::stringstream content;
    content << "--" << GetMultipartBoundary() << "\r\n"
            << "Content-Disposition: form-data; name=\"" << name << "\"\r\n"
            << "Content-Type: application/json\r\n\r\n"
            << (std::string)JSONString(data.serialize()) << "\r\n";

    return content.str();
}

ServerSync::ServerSync() : _characterTrusted(true), _stashTrusted(true)
{
    EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_READ, &ServerSync::OnDirectRead);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD, &ServerSync::OnWorldPreLoad);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD, &ServerSync::OnWorldUnload);
}

ServerSync& ServerSync::GetInstance()
{
    static ServerSync instance;
    return instance;
}

bool ServerSync::IsClientTrusted()
{
    ServerSync& sync = ServerSync::GetInstance();
    return ((sync._characterTrusted) && (sync._stashTrusted));
}

uint32_t ServerSync::GetParticipantID(bool hardcore)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "profile";

        std::string branch = client.GetBranch();
        endpoint.AddParam("branch", client.GetBranch());

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
    ServerSync& sync = ServerSync::GetInstance();

    if (participantID == 0)
        participantID = client.GetParticipantID();

    CharacterIDRef ref = { playerName, participantID };
    if (sync._characterIDCache.count(ref) > 0)
    {
        return sync._characterIDCache[ref];
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
                    sync._characterIDCache[ref] = (uint32_t)characterID.as_integer();
                    return sync._characterIDCache[ref];
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
                result._checksum = JSONString(responseBody[U("currentChecksum")].serialize());
                result._modifiedTime = responseBody[U("modifiedOn")].as_number().to_uint64();
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
                result._checksum = JSONString(responseBody[U("currentChecksum")].serialize());
                result._modifiedTime = responseBody[U("modifiedOn")].as_number().to_uint64();
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

void ServerSync::SnapshotCharacterMetadata(const std::wstring& playerName)
{
    ServerSync& sync = ServerSync::GetInstance();
    FileMetadata metadata(GameAPI::GetPlayerSaveFile(playerName));
    if (sync._characterMetadata.IsEmpty())
        sync._characterMetadata = metadata;

    if ((sync._characterTrusted) && (metadata == sync._lastTrustedCharacterMetadata))
        sync._lastTrustedCharacterMetadata = metadata;
}


void ServerSync::SnapshotStashMetadata(const std::string& modName, bool hardcore)
{
    ServerSync& sync = ServerSync::GetInstance();
    FileMetadata metadata(GameAPI::GetTransferStashPath(modName, hardcore)); 

    if (sync._stashMetadata.IsEmpty())
        sync._stashMetadata = metadata;

    if ((sync._characterTrusted) && (metadata == sync._lastTrustedCharacterMetadata))
        sync._lastTrustedCharacterMetadata = metadata;
}

void ServerSync::PostCharacterUpload(bool newPlayer)
{
    Client& client = Client::GetInstance();
    ServerSync& sync = ServerSync::GetInstance();

    if ((client.IsParticipatingInSeason()) || (newPlayer))
    {
        std::filesystem::path characterPath = GameAPI::GetPlayerFolder(sync._characterName);
        std::filesystem::path characterSavePath = GameAPI::GetPlayerSaveFile(sync._characterName);

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
        requestJSON[U("seasonParticipantId")] = client.GetParticipantID();
        requestJSON[U("participantCharacterId")] = (newPlayer) ? 0 : GetCharacterID(sync._characterName);

        FileMetadata oldMetadata = sync._characterMetadata;
        FileMetadata newMetadata(characterSavePath);
        requestJSON[U("lastChecksum")] = JSONString(oldMetadata._checksum);
        requestJSON[U("currentChecksum")] = JSONString(newMetadata._checksum);
        requestJSON[U("lastModifiedOn")] = oldMetadata._modifiedTime;
        requestJSON[U("modifiedOn")] = newMetadata._modifiedTime;
        requestJSON[U("clientTrusted")] = IsClientTrusted();

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("character", requestJSON);
        requestBody += GetMultipartFileData("file", characterSavePath);
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character";
            endpoint.AddParam("newCharacter", newPlayer);

            web::http::http_request request(web::http::methods::POST);

            request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            ServerSync::GetInstance()._backgroundTasks.run([endpoint, request]()
            {
                web::http::client::http_client httpClient((utility::string_t)endpoint);
                return httpClient.request(request).then([](web::http::http_response response) {
                    if (response.status_code() == web::http::status_codes::OK)
                    {
                        Client::GetInstance().UpdateSeasonStanding();
                    }
                    else
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "While uploading character data: Server responded with status code " + std::to_string(response.status_code()));
                    }
                });
            });

        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character data: %", ex.what());
        }

        if (sync._characterTrusted)
        {
            sync._lastTrustedCharacterMetadata = newMetadata;
        }
    }

    sync._characterMetadata.Clear();
  

    return;
}

void ServerSync::PostStashUpload()
{
    Client& client = Client::GetInstance();
    ServerSync& sync = ServerSync::GetInstance();
    if (client.IsParticipatingInSeason())
    {
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();
        std::filesystem::path stashPath = GameAPI::GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer));

        FileMetadata oldMetadata = sync._stashMetadata;
        FileMetadata newMetadata(stashPath);

        web::json::value requestJSON;
        requestJSON[U("lastChecksum")] = JSONString(oldMetadata._checksum);
        requestJSON[U("currentChecksum")] = JSONString(newMetadata._checksum);
        requestJSON[U("lastModifiedOn")] = oldMetadata._modifiedTime;
        requestJSON[U("modifiedOn")] = newMetadata._modifiedTime;
        requestJSON[U("clientTrusted")] = IsClientTrusted();

        std::string requestBody;
        requestBody += "\r\n";
        requestBody += GetMultipartJSONData("sharedstash", requestJSON);
        requestBody += GetMultipartFileData("file", stashPath);
        requestBody += "--" + GetMultipartBoundary() + "--\r\n";

        try
        {
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "shared-stash";

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);

            request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() != web::http::status_codes::OK)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash: %", ex.what());
        }

        if (sync._stashTrusted)
        {
            sync._lastTrustedStashMetadata = newMetadata;
        }
    }
    sync._stashMetadata.Clear();
}

void ServerSync::PostCloudStashUpload()
{
    Client& client = Client::GetInstance();
    ServerSync& sync = ServerSync::GetInstance();
    if (client.IsParticipatingInSeason())
    {
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();

        std::filesystem::path stashPath = GameAPI::GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer));
        if (stashPath.empty())
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine shared stash path for mod \"%\"", modName);
            return;
        }

        SharedStash stashData;
        if (!stashData.ReadFromFile(stashPath))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not load shared stash data from file");
            return;
        }

        web::json::value stashJSON = stashData.ToJSON();
        web::json::array stashTabs = stashJSON[U("Tabs")].as_array();
        if (stashTabs.size() >= 6)
        {
            web::json::array transferItems = stashTabs[5][U("Items")].as_array();
            if (transferItems.size() > 0)
            {
                uint32_t index = 0;
                web::json::value requestBody = web::json::value::array();
                for (auto it = transferItems.begin(); it != transferItems.end(); ++it)
                {
                    requestBody[index] = *it;
                    requestBody[index].erase(U("unknown1"));
                    requestBody[index].erase(U("unknown2"));
                    requestBody[index].erase(U("X"));
                    requestBody[index].erase(U("Y"));
                    index++;
                }

                try
                {
                    URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "stash";
                    endpoint.AddParam("branch", client.GetBranch());
                    endpoint.AddParam("clientTrusted", IsClientTrusted());

                    web::http::client::http_client httpClient((utility::string_t)endpoint);
                    web::http::http_request request(web::http::methods::POST);
                    request.set_body(requestBody);

                    std::string bearerToken = "Bearer " + client.GetAuthToken();
                    request.headers().add(U("Authorization"), bearerToken.c_str());

                    web::http::http_response response = httpClient.request(request).get();
                    if (response.status_code() == web::http::status_codes::OK)
                    {
                        Stash::StashTab* stashTab = stashData.GetStashTab(5);
                        if (stashTab)
                        {
                            // TODO: Possibly check the data returned by the response in case not all of the items could be stored
                            stashTab->GetItemList().clear();
                            stashData.WriteToFile(stashPath);
                            GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
                        }
                        else
                        {
                            throw std::runtime_error("Could not retrieve stash tab data from save file");
                        }
                    }
                    else if (response.status_code() == web::http::status_codes::BadRequest)
                    {
                        GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
                    }
                    else
                    {
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                    }
                }
                catch (const std::exception& ex)
                {
                    GameAPI::DisplayUINotification("tagGDLeagueStorageFailure");
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash items: %", ex.what());
                }
            }
        }
    }
}

void ServerSync::UploadCharacterData(bool buffered)
{
    if (buffered)
        ThreadManager::CreatePeriodicThread("character_save", 1000, 0, 5000, &ServerSync::PostCharacterUpload, false);
    else
        PostCharacterUpload();
}

void ServerSync::UploadStashData()
{
    PostCloudStashUpload();
    PostStashUpload();
}

bool ServerSync::DownloadCharacterData(const std::wstring& playerName, uint32_t participantID)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character" / playerName / "file";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            std::filesystem::path savePath = GameAPI::GetPlayerSaveFile(playerName);
            std::filesystem::path tempPath = savePath;
            tempPath += ".tmp";

            concurrency::streams::ostream fileStream = concurrency::streams::fstream::open_ostream(tempPath).get();
            concurrency::streams::istream body = response.body();

            size_t bytesRead = 0;
            do
            {
                bytesRead = body.read(fileStream.streambuf(), 1024).get();
            }
            while (bytesRead > 0);

            fileStream.close().wait();
            std::filesystem::rename(tempPath, savePath);
            return true;
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
    return false;
}

void ServerSync::RefreshCharacterMetadata(const std::wstring& playerName, uint32_t participantID, web::json::value& characterJSON)
{
   
    ServerSync& sync = ServerSync::GetInstance();
    web::json::value characterInfo = web::json::value::object();
    characterInfo[U("name")] = characterJSON[U("HeaderBlock")][U("Name")];
    characterInfo[U("level")] = characterJSON[U("HeaderBlock")][U("Level")];
    characterInfo[U("timePlayed")] = characterJSON[U("StatsBlock")][U("PlayedTime")];

    web::json::value requestJSON;
    requestJSON[U("characterData")] = web::json::value::object();
    requestJSON[U("characterData")][U("characterInfo")] = characterInfo;
    requestJSON[U("seasonParticipantId")] = participantID;
    requestJSON[U("participantCharacterId")] = GetCharacterID(playerName, participantID);

    FileMetadata saveMetadata(GameAPI::GetPlayerSaveFile(playerName));
    requestJSON[U("currentChecksum")] = JSONString(saveMetadata._checksum);
    requestJSON[U("modifiedOn")] = saveMetadata._modifiedTime;
    requestJSON[U("clientTrusted")] = IsClientTrusted();

    std::string requestBody;
    requestBody += "\r\n";
    requestBody += GetMultipartJSONData("character", requestJSON);
    requestBody += "--" + GetMultipartBoundary() + "--\r\n";

    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "character";

        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());


        ServerSync::GetInstance()._backgroundTasks.run([endpoint, request]()
        {
                web::http::client::http_client httpClient((utility::string_t)endpoint);
                return httpClient.request(request).then([](web::http::http_response response) {
                    if (response.status_code() != web::http::status_codes::OK)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "While uploading character metadata: Server responded with status code " + std::to_string(response.status_code()));
                    }
                });
        });
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload character metadata: %", ex.what());
    }
}

void ServerSync::SyncCharacterData(const std::filesystem::path& filePath)
{
    ServerSync& sync = ServerSync::GetInstance();
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

            if ((serverMetadata != sync._characterMetadata) && (clientMetadata != sync._lastTrustedCharacterMetadata))
            {
                uint32_t timePlayed = characterJSON[U("StatsBlock")][U("PlayedTime")].as_integer();
                // TODO: Eliminate this case by having the server create the initial character data
                if (timePlayed == 0)
                {
                    PostCharacterUpload(true);
                    sync._characterTrusted = true;
                    sync._lastTrustedCharacterMetadata = clientMetadata;
                }
                else if ((DownloadCharacterData(playerName, participantID)) && (sync._characterTrusted))
                {
                    RefreshCharacterMetadata(playerName, participantID, characterJSON);
                    sync._lastTrustedCharacterMetadata = clientMetadata;
                }
                else
                {
                    sync._characterTrusted = false;
                }
            }
            sync._characterName = playerName;
            return;
        }
    }
    sync._characterTrusted = false;
}

bool ServerSync::DownloadStashData(const std::string& modName, bool hardcore, uint32_t participantID)
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
            std::filesystem::path savePath = GameAPI::GetTransferStashPath(modName, hardcore);
            std::filesystem::path tempPath = savePath;
            tempPath += ".tmp";

            concurrency::streams::ostream fileStream = concurrency::streams::fstream::open_ostream(tempPath).get();
            concurrency::streams::istream body = response.body();

            size_t bytesRead = 0;
            do
            {
                bytesRead = body.read(fileStream.streambuf(), 1024).get();
            }
            while (bytesRead > 0);

            fileStream.close().wait();

            std::filesystem::rename(tempPath, savePath);
            return true;
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
    return false;
}

void ServerSync::RefreshStashMetadata(const std::string& modName, bool hardcore, uint32_t participantID)
{
    ServerSync& sync = ServerSync::GetInstance();

    web::json::value requestJSON;
    FileMetadata saveMetadata(GameAPI::GetTransferStashPath(modName, hardcore));
    requestJSON[U("currentChecksum")] = JSONString(saveMetadata._checksum);
    requestJSON[U("modifiedOn")] = saveMetadata._modifiedTime;
    requestJSON[U("clientTrusted")] = IsClientTrusted();

    std::string requestBody;
    requestBody += "\r\n";
    requestBody += GetMultipartJSONData("sharedstash", requestJSON);
    requestBody += "--" + GetMultipartBoundary() + "--\r\n";

    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "shared-stash";

        
        web::http::http_request request(web::http::methods::POST);

        request.set_body(requestBody, "multipart/form-data; boundary=" + GetMultipartBoundary());

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        ServerSync::GetInstance()._backgroundTasks.run([endpoint, request, saveMetadata]() {
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            return httpClient.request(request).then([saveMetadata](web::http::http_response response)
            {
                if (response.status_code() == web::http::status_codes::OK)
                {
                    ServerSync::GetInstance()._lastTrustedStashMetadata = saveMetadata;
                }
                else
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "While uploading stash metadata: Server responded with status code " + std::to_string(response.status_code()));
                }
            });
        });
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash metadata: %", ex.what());
    }
}

void ServerSync::SyncStashData(const std::filesystem::path& filePath, bool hardcore)
{
    ServerSync& sync = ServerSync::GetInstance();
    std::filesystem::path parentPath = filePath.parent_path();
    std::string modName = parentPath.filename().string();
    if (modName == "save")
        modName = {};

    uint32_t participantID = ServerSync::GetParticipantID(hardcore);
    FileMetadata clientMetadata(filePath);
    FileMetadata serverMetadata = GetServerStashMetadata(participantID);

    if ((serverMetadata != sync._stashMetadata) && (clientMetadata != sync._lastTrustedStashMetadata))
    {
        if ((DownloadStashData(modName, hardcore, participantID)) && (sync._stashTrusted))
        {
            RefreshStashMetadata(modName, hardcore, participantID);
            sync._lastTrustedStashMetadata = clientMetadata;
        }
        else
        {
            sync._stashTrusted = false;
        }
    }

}

void ServerSync::OnDirectRead(void* data)
{
    Client& client = Client::GetInstance();
    ServerSync& sync = ServerSync::GetInstance();
    std::filesystem::path filePath = *(std::string*)data;
    if ((filePath.filename() == "player.gdc") && (client.IsInProductionBranch()))
    {
        SyncCharacterData(filePath);
    }
}

void ServerSync::OnWorldPreLoad(void* data)
{
    ServerSync& sync = ServerSync::GetInstance();
    std::string mapName = (const char*)data;
    if (mapName == "levels/mainmenu/mainmenu.map")
    {
        // TODO: Sync all characters from the server, including those that aren't currently present (+quest/FOW/etc. data)
    }
    else
    {
        sync._stashTrusted = true;
        sync._characterTrusted = true;
        sync._lastTrustedCharacterMetadata.Clear();
        sync._lastTrustedStashMetadata.Clear();
    }
}

void ServerSync::OnWorldUnload(void* data)
{
    Client& client = Client::GetInstance();
    PostCharacterUpload();
}

void ServerSync::WaitBackgroundComplete()
{
    ServerSync::GetInstance()._backgroundTasks.wait();
}