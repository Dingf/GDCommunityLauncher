#include <future>
#include <string>
#include "SeasonClient.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetCharacters(uint32_t requestID, uint32_t participantID)
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

std::string HandleWriteGetCharacterData(uint32_t requestID, uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterData" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetCharacterFile(uint32_t requestID, uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName }
        }}
    };
    return request.dump();
}

std::string HandleWriteDeleteCharacter(uint32_t requestID, uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "DeleteParticipantCharacter" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

void HandleReadGetCharacters(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadGetCharacterData(json response, uint32_t participantID, std::wstring characterName)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get character data: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetCharacterFile(json response, uint32_t participantID, std::wstring characterName)
{
    // TODO
}

void HandleReadDeleteCharacter(json response, uint32_t participantID, std::wstring characterName)
{
    // TODO
}