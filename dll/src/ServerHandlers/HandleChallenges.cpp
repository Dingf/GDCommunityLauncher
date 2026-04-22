#include <string>
#include "ChallengeManager.h"
#include "HTTP.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetChallenges(uint32_t requestID, uint32_t& seasonID, uint32_t& participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantChallenges" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonId", seasonID },
            { "SeasonParticipantId", participantID }
        }}
    };
    return request.dump();
}

std::string HandleWriteGetSeasonChallenges(uint32_t requestID, uint32_t& seasonID)
{
    json request =
    {
        { "RequestName", "GetSeasonChallenges" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonId", seasonID }
        }}
    };
    return request.dump();
}

void HandleReadGetChallenges(const json& response, uint32_t seasonID, uint32_t participantID)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status == HTTP_STATUS_OK)
    {
        const json& challenges = response.at("Data");
        for (const json& challenge : challenges)
        {
            uint32_t challengeID = challenge.at("SeasonChallengeId").get<uint32_t>();
            if (!spChallengeManager->SetChallengeStatus(seasonID, challengeID, CHALLENGE_STATUS_COMPLETE))
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Completed participant challenge (ID: %) was not found in challenge list", challengeID);
            }
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get participant challenges: %", response.at("ErrorMessage"));
    }
}

void HandleReadGetSeasonChallenges(const json& response, uint32_t seasonID)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status == HTTP_STATUS_OK)
    {
        const json& challenges = response.at("Data");
        for (const json& challenge : challenges)
        {
            uint32_t challengeID = challenge.at("SeasonChallengeId").get<uint32_t>();
            if (!spChallengeManager->GetChallenge(seasonID, challengeID))
            {
                Challenge newChallenge;
                newChallenge._name = challenge.at("ChallengeName").get<std::string>();
                newChallenge._challengeID = challengeID;
                if (challenge.at("MaxLevel").is_null())
                {
                    newChallenge._maxLevel = 0;
                }
                else
                {
                    newChallenge._maxLevel = challenge.at("MaxLevel").get<uint32_t>();
                }
                newChallenge._status = CHALLENGE_STATUS_INCOMPLETE;
                newChallenge._points = challenge.at("PointValue").get<uint32_t>();
                newChallenge._active = challenge.at("Active").get<bool>();
                newChallenge._category = challenge.at("ChallengeCategoryId").get<uint32_t>();

                if (newChallenge._category == 0)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Challenge \"%\" (ID: %) has unknown category: %", newChallenge._name, challengeID);
                }
                // TODO Change once Hidden has its own field or is otherwise defined
                else if (newChallenge._category == 10)
                {
                    newChallenge._status = CHALLENGE_STATUS_HIDDEN;
                }

                std::string difficultyNames = challenge.at("Difficulties").get<std::string>();
                newChallenge._difficultyRaw = difficultyNames;
                for (const auto& difficultyEntry : spChallengeManager->GetChallengeDifficulties())
                {
                    if (difficultyNames.find(difficultyEntry.first) != std::string::npos)
                    {
                        newChallenge._difficultyBitMask |= difficultyEntry.second;
                    }
                }
                if (newChallenge._difficultyBitMask == 0)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Challenge \"%\" (ID: %) has unknown difficulty: %", newChallenge._name, challengeID, difficultyNames);
                }

                spChallengeManager->AddChallenge(seasonID, newChallenge);
            }
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get season challenges: %", response.at("ErrorMessage"));
    }
}