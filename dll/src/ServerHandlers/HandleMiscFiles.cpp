#include <string>
#include "GameAPI/Difficulty.h"
#include "JSON.h"

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

void HandleReadGetTagFile(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadSaveTagFile(json response, uint32_t participantID, std::string base64Data)
{
    // TODO
}

void HandleReadGetTransmuteFile(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadSaveTransmuteFile(json response, uint32_t participantID, std::string base64Data)
{
    // TODO
}

void HandleReadGetFormulasFile(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadSaveFormulasFile(json response, uint32_t participantID, std::string base64Data)
{
    // TODO
}

void HandleReadGetQuestFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    // TODO
}

void HandleReadSaveQuestFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    // TODO
}

void HandleReadGetConversationFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    // TODO
}

void HandleReadSaveConversationFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    // TODO
}

void HandleReadGetMapFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    // TODO
}

void HandleReadSaveMapFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    // TODO
}

void HandleReadGetFOWFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty)
{
    // TODO
}

void HandleReadSaveFOWFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data)
{
    // TODO
}