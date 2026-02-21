#include <string>
#include "GameAPI/Difficulty.h"
#include "JSON.h"

std::string HandleWriteGetTagFile(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTagFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveTagFile(uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantTagFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetTransmuteFile(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTransmutes" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveTransmuteFile(uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantTransmutes" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetFormulasFile(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantFormulas" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveFormulasFile(uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantFormulas" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetQuestFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterQuestFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveQuestFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterQuestFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetConversationFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterConversationsFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveConversationFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterConversationsFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetMapFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterMapDatFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveMapFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterMapDatFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteGetFOWFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    json request = 
    {
        { "RequestName", "GetParticipantCharacterMapFowFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveFOWFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantCharacterMapFowFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", characterName },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

void HandleReadGetTagFile(json response)
{
    // TODO
}

void HandleReadSaveTagFile(json response)
{
    // TODO
}

void HandleReadGetTransmuteFile(json response)
{
    // TODO
}

void HandleReadSaveTransmuteFile(json response)
{
    // TODO
}

void HandleReadGetFormulasFile(json response)
{
    // TODO
}

void HandleReadSaveFormulasFile(json response)
{
    // TODO
}

void HandleReadGetQuestFile(json response)
{
    // TODO
}

void HandleReadSaveQuestFile(json response)
{
    // TODO
}

void HandleReadGetConversationFile(json response)
{
    // TODO
}

void HandleReadSaveConversationFile(json response)
{
    // TODO
}

void HandleReadGetMapFile(json response)
{
    // TODO
}

void HandleReadSaveMapFile(json response)
{
    // TODO
}

void HandleReadGetFOWFile(json response)
{
    // TODO
}

void HandleReadSaveFOWFile(json response)
{
    // TODO
}