#include <future>
#include <string>
#include "GameAPI.h"
#include "DllClient.h"
#include "ServerCache.h"
#include "StringConvert.h"
#include "HTTP.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetCharacters(uint32_t requestID, uint32_t& participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacters" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetCharacterData(uint32_t requestID, uint32_t& participantID, std::wstring& characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterData" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", WideToChar(characterName) }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetCharacterFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", WideToChar(characterName) }
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveCharacterFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, std::string& base64Data)
{
    uint32_t characterID = spCache->GetCharacterID(characterName);
    json request =
    {
        { "RequestName", "SaveCharacterFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "ParticipantCharacterId", characterID },
            { "Branch", spClient->GetBranchName() }
        }},
        { "File", base64Data }
    };
    base64Data.clear();
    return request.dump();
}

std::string HandleWriteDeleteCharacter(uint32_t requestID, uint32_t& participantID, std::wstring& characterName)
{
    json request = 
    {
        { "RequestName", "DeleteParticipantCharacter" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", WideToChar(characterName) },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

void HandleReadGetCharacters(const json& response, uint32_t participantID)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status != HTTP_STATUS_OK)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character list: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetCharacterData(const json& response, uint32_t participantID, std::wstring characterName)
{
    try
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            const json& data = response.at("Data");
            uint32_t characterID = data.at("ParticipantCharacterId").get<uint32_t>();
            spCache->SetCharacterID(characterName, participantID, characterID);
        }
        else if (status != HTTP_STATUS_NO_CONTENT)
        {
            throw std::runtime_error(response.at("ErrorMessage"));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character data: %", ex.what());
    }
}

void HandleReadGetCharacterFile(const json& response, uint32_t participantID, std::wstring characterName)
{
    try
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            const json& file = response.at("File");
            std::string base64Data = file.get<std::string>();
            std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

            std::filesystem::path filePath = GameAPI::GetPlayerSaveFile(characterName);
            FileWriter writer(&binaryData[0], binaryData.size());
            writer.WriteToFile(filePath);

            spCache->SetCharacterData(characterName, &binaryData[0], binaryData.size());
        }
        else if (status != HTTP_STATUS_NO_CONTENT)
        {
            throw std::runtime_error(response.at("ErrorMessage"));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load character file: %", ex.what());
    }
}

void HandleReadSaveCharacterFile(const json& response, uint32_t participantID, std::wstring characterName, std::string base64Data)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status != HTTP_STATUS_OK)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save character file: %", response.at("ErrorMessage"));
    }
}

void HandleReadDeleteCharacter(const json& response, uint32_t participantID, std::wstring characterName)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status != HTTP_STATUS_OK)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to delete character file: %", response.at("ErrorMessage"));
    }
}