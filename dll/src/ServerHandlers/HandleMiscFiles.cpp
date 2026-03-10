#include <string>
#include <boost/algorithm/string.hpp>
#include "GameAPI.h"
#include "ServerCache.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "StringConvert.h"
#include "Quest.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetTagFile(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTagFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveTagFile(uint32_t requestID, uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantTagFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetTransmuteFile(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTransmutes" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveTransmuteFile(uint32_t requestID, uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantTransmutes" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetFormulasFile(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantFormulas" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveFormulasFile(uint32_t requestID, uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantFormulas" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetQuestFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterQuestFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveQuestFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterQuestFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetConversationFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterConversationsFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveConversationFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterConversationsFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetMapFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterMapDatFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveMapFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterMapDatFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetFOWFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterMapFowFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveFOWFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterMapFowFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

void HandleReadGetTagFile(const json& response, uint32_t participantID)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        FileReader reader(&binaryData[0], binaryData.size());

        uint32_t count = reader.ReadInt32();
        reader.ReadInt32();     // This is the size of the string contents, but not really needed here

        void* mainPlayer = GameAPI::GetMainPlayer();
        if (!mainPlayer)
            throw std::runtime_error("Main player is not active");

        for (size_t i = 0; i < count; ++i)
        {
            std::string tokenString = reader.ReadString();
            GameAPI::BestowTokenNow(mainPlayer, tokenString);
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared tags file: %", ex.what());
    }
}

void HandleReadSaveTagFile(const json& response, uint32_t participantID, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save shared tags file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetTransmuteFile(const json& response, uint32_t participantID)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(GameAPI::GetTransmutesPath(spCache->IsParticipantHardcore(participantID)));
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load transmutes file: %", response.at("ErrorMessage"));
    }
}

void HandleReadSaveTransmuteFile(const json& response, uint32_t participantID, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save shared transmutes file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetFormulasFile(const json& response, uint32_t participantID)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(GameAPI::GetFormulasPath(spCache->IsParticipantHardcore(participantID)));
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load shared formulas file: %", response.at("ErrorMessage"));
    }
}

void HandleReadSaveFormulasFile(const json& response, uint32_t participantID, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save shared formulas file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetQuestFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        // If the player is in-game, then we're calling this to load the quest states from the server
        // Otherwise, sync the quest file with the server
        if (void* mainPlayer = GameAPI::GetMainPlayer())
        {
            Quest questData;
            if (!questData.ReadFromBuffer(&binaryData[0], binaryData.size()))
                throw std::runtime_error("Could not read quest data from server");

            GameAPI::ClearPlayerTokens(mainPlayer);
            for (const auto& token : questData._tokensBlock._questTokens)
            {
                // Skip loading these, as they will be loaded from the tags file
                if (boost::algorithm::starts_with(token, "GDIP_"))
                    continue;

                GameAPI::BestowTokenNow(mainPlayer, token);
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
        else
        {
            std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "quests.gdd";
            FileWriter writer(&binaryData[0], binaryData.size());
            writer.WriteToFile(filePath);
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character quest file: %", ex.what());
    }
}

void HandleReadSaveQuestFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save character quest file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetConversationFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "conversations.gdd";
        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(filePath);
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character conversation file: %", ex.what());
    }
}

void HandleReadSaveConversationFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save character conversation file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetMapFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "map.dat";
        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(filePath);
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character map file: %", ex.what());
    }
}

void HandleReadSaveMapFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save character map file: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetFOWFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "map.fow";
        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(filePath);
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character FOW file: %", ex.what());
    }
}

void HandleReadSaveFOWFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save character FOW file: %", response.at("ErrorMessage"));
    }
}