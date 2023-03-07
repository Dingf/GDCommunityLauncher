#include <filesystem>
#include <cpprest/http_client.h>
#include "Character.h"
#include "Quest.h"
#include "ClientHandlers.h"
#include "URI.h"
#include "MD5.h"

const std::unordered_map<GameAPI::Difficulty, const utility::char_t*> difficultyTagLookup =
{
    { GameAPI::GAME_DIFFICULTY_NORMAL,   U("normalQuestTags") },
    { GameAPI::GAME_DIFFICULTY_ELITE,    U("eliteQuestTags") },
    { GameAPI::GAME_DIFFICULTY_ULTIMATE, U("ultimateQuestTags") },
};

std::filesystem::path GetCharacterPath(const std::wstring& playerName)
{
    std::filesystem::path result = std::filesystem::path(GameAPI::GetBaseFolder()) / "save" / "user" / "_";
    result += playerName;
    return result;
}

uint32_t GetCharacterID(const std::wstring& playerName)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character" / playerName;
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
        return 0;
    }
}

void PostCharacterDataUpload(const std::wstring& playerName, const std::string& prevChecksum)
{
    Client& client = Client::GetInstance();

    // Shouldn't happen, but if player name is empty we can't do anything here
    if (playerName.empty())
        return;

    uint32_t characterID = GetCharacterID(playerName);
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    std::filesystem::path characterPath = GetCharacterPath(playerName);
    std::filesystem::path characterSavePath = characterPath / "player.gdc";

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

    web::json::value requestBody;
    requestBody[U("characterData")] = web::json::value::object();
    requestBody[U("characterData")][U("characterInfo")] = characterInfo;
    requestBody[U("characterData")][U("questInfo")] = questInfo;
    requestBody[U("seasonParticipantId")] = client.GetParticipantID();
    requestBody[U("participantCharacterId")] = characterID;
    requestBody[U("lastChecksum")] = JSONString(prevChecksum);
    requestBody[U("currentChecksum")] = JSONString(GenerateFileMD5(characterSavePath));

    pplx::task<void> task = pplx::create_task([requestBody]()
    {
        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "character";

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);
            request.set_body(requestBody);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                client.UpdateSeasonStanding();
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

void HandleSaveNewFormatData(void* _this, void* writer)
{
    typedef void(__thiscall* SaveNewFormatDataProto)(void*, void*);

    SaveNewFormatDataProto callback = (SaveNewFormatDataProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    if (callback)
    {
        std::string prevChecksum;

        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            std::filesystem::path characterPath = GetCharacterPath(client.GetActiveCharacterName());
            std::filesystem::path characterSavePath = characterPath / "player.gdc";
            prevChecksum = GenerateFileMD5(characterSavePath);
        }

        callback(_this, writer);

        const SeasonInfo* seasonInfo = client.GetActiveSeason();
        if ((!EngineAPI::IsMultiplayer()) && (client.IsParticipatingInSeason()))
        {
            pplx::create_task([prevChecksum]()
            {
                Client& client = Client::GetInstance();
                PostCharacterDataUpload(client.GetActiveCharacterName(), prevChecksum);
            });
        }
    }
}