#include <string>
#include <filesystem>
#include "GameAPI.h"
#include "ServerCache.h"
#include "ServerHandler.h"
#include "HTTP.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetChecksum(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty, std::filesystem::path& filePath)
{
    json request = 
    {
        { "RequestName", "GetFileChecksum" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "CharacterName", WideToChar(characterName) },
            { "Difficulty", GameAPI::GetGameDifficultyName(difficulty) },
            { "FileName", filePath.filename().string() }
        }}
    };
    return request.dump();
}

void HandleReadGetChecksum(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::filesystem::path filePath)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if ((status != HTTP_STATUS_OK) && (status != HTTP_STATUS_NOT_FOUND))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read checksum for %: %", filePath.filename().string(), response.at("ErrorMessage"));
    }
}