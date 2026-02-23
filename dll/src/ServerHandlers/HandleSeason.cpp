#include <string>
#include "ServerCache.h"
#include "SeasonClient.h"
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

std::string HandleWriteAddParticipant(uint32_t requestID, uint32_t seasonID)
{
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

void HandleReadGetSeasons(json response)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        spClient->_seasons.clear();

        json seasons = response.at("Data");
        for (json season : seasons)
        {
            spClient->_seasons.push_back({});
            auto& seasonInfo = spClient->_seasons.back();

            seasonInfo._seasonID = season.at("SeasonId").get<uint32_t>();
            seasonInfo._seasonType = season.at("SeasonTypeId").get<SeasonType>();
            seasonInfo._displayName = season.at("DisplayName").get<std::string>();
            seasonInfo._participationToken = season.at("ParticipationTag").get<std::string>();
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to add season participant: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetPoints(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadGetTradeNotifications(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadAddParticipant(json response, uint32_t seasonID)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        json data = response.at("Data");
        uint32_t participantID = data.at("SeasonParticipantId").get<uint32_t>();
        SeasonType seasonType = data.at("Season").at("SeasonTypeId").get<SeasonType>();

        spServerCache->SetParticipantID(seasonType == SEASON_TYPE_HC_SSF, participantID);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to add season participant: %", response.at("ErrorMessage"));
    }
}