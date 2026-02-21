#include <string>
#include "SeasonClient.h"
#include "JSON.h"

std::string HandleWriteGetCharacters(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacters" },
        { "Arguments", {
            { "SeasonParticipantId", participantID }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetCharacterData(uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterData" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetCharacterFile(uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "GetCharacterFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName }
        }}
    };
    return request.dump();
}

std::string HandleWriteDeleteCharacter(uint32_t participantID, std::wstring characterName)
{
    json request = 
    {
        { "RequestName", "DeleteParticipantCharacter" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

void HandleReadGetCharacters(json response)
{
    // TODO
}

void HandleReadGetCharacterData(json response)
{
    // TODO
}

void HandleReadGetCharacterFile(json response)
{
    // TODO
}

void HandleReadDeleteCharacter(json response)
{
    // TODO
}