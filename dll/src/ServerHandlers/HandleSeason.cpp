#include <future>
#include <string>
#include "GameAPI.h"
#include "ChatAPI.h"
#include "ServerCache.h"
#include "DllClient.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetSeasons(uint32_t requestID)
{
    json request = 
    {
        { "RequestName", "GetSeasons" },
        { "RequestId", requestID },
        { "Arguments", {
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetPoints(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantPoints" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetTradeNotifications(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetNewTradeNotificationCount" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

std::string HandleWriteAddParticipant(uint32_t requestID, bool hardcore)
{
    uint32_t seasonID = 0;
    if (const SeasonInfo* season = spClient->GetSeasonByType(hardcore))
        seasonID = season->_seasonID;

    json request = 
    {
        { "RequestName", "AddParticipant" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonId", seasonID }
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveTag(uint32_t requestID, uint32_t participantID, std::string tagName, uint32_t level, GameAPI::Difficulty difficulty)
{
    json request =
    {
        { "RequestName", "SaveParticipantTag" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "TagName", tagName },
            { "Level", level },
            { "CurrentDifficulty", GameAPI::GetGameDifficultyName(difficulty) },
        }}
    };
    return request.dump();
}

void HandleReadGetSeasons(const json& response)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        auto& seasonList = spClient->GetSeasonList();
        seasonList.clear();

        const json& seasons = response.at("Data");
        for (const json& season : seasons)
        {
            seasonList.push_back({});
            auto& seasonInfo = seasonList.back();

            seasonInfo._seasonID = season.at("SeasonId").get<uint32_t>();
            seasonInfo._seasonType = season.at("SeasonTypeId").get<SeasonType>();
            seasonInfo._displayName = season.at("DisplayName").get<std::string>();
            seasonInfo._participationToken = season.at("ParticipationTag").get<std::string>();
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get season data: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetPoints(const json& response, uint32_t participantID)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        const json& data = response.at("Data");
        spClient->SetPoints(data.at("PointTotal").get<uint32_t>());
        spClient->SetRank(data.at("Rank").get<uint32_t>());
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update season standing: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetTradeNotifications(const json& response, uint32_t participantID)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        uint32_t notificationCount = response.at("Data").get<uint32_t>();
        if (notificationCount > 0)
        {
            std::wstring message = L"You have " + std::to_wstring(notificationCount) + L" new trade notification";
            if (notificationCount > 1)
            {
                message += L"s";
            }
            message += L".";
            GameAPI::AddChatMessage(L"Server", message, static_cast<uint8_t>(ChatAPI::CHAT_TYPE_SYSTEM));
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve new trade notifications: %", response.at("ErrorMessage"));
    }
}

void HandleReadAddParticipant(const json& response, bool hardcore)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        const json& data = response.at("Data");
        spCache->SetParticipantID(hardcore, data.at("SeasonParticipantId").get<uint32_t>());
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to add season participant: %", response.at("ErrorMessage"));
    }
}

void HandleReadSaveTag(const json& response, uint32_t participantID, std::string tagName, uint32_t level, GameAPI::Difficulty difficulty)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        const json& data = response.at("Data");
        spClient->SetPoints(data.at("PointTotal").get<uint32_t>());
        spClient->SetRank(data.at("Rank").get<uint32_t>());
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to save season tag: %", response.at("ErrorMessage"));
    }
}