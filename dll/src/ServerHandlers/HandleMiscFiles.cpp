#include <string>
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

        bool hardcore = spCache->IsParticipantHardcore(participantID);
        spCache->SetTagsData(hardcore, &binaryData[0], binaryData.size());
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

        bool hardcore = spCache->IsParticipantHardcore(participantID);
        writer.WriteToFile(GameAPI::GetTransmutesPath(hardcore));

        spCache->SetTransmutesData(hardcore, &binaryData[0], binaryData.size());
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

        bool hardcore = spCache->IsParticipantHardcore(participantID);
        writer.WriteToFile(GameAPI::GetFormulasPath(hardcore));

        spCache->SetFormulasData(hardcore, &binaryData[0], binaryData.size());
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

        std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "quests.gdd";
        FileWriter writer(&binaryData[0], binaryData.size());
        writer.WriteToFile(filePath);

        spCache->SetQuestData(characterName, difficulty, &binaryData[0], binaryData.size());
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

        spCache->SetConversationsData(characterName, difficulty, &binaryData[0], binaryData.size());
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

        spCache->SetMapData(characterName, difficulty, &binaryData[0], binaryData.size());
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

        spCache->SetFOWData(characterName, difficulty, &binaryData[0], binaryData.size());
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