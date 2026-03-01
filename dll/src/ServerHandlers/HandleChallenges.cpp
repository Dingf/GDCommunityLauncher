#include <string>
#include "ChallengeManager.h"
#include "JSON.h"
#include "Log.h"

const std::unordered_map<std::string, ChallengeDifficulty> challengeDifficultyMap =
{
    { "Normal", CHALLENGE_DIFFICULTY_NORMAL },
    { "Elite", CHALLENGE_DIFFICULTY_ELITE },
    { "Ultimate", CHALLENGE_DIFFICULTY_ULTIMATE }
};

std::string HandleWriteGetChallenges(uint32_t requestID, uint32_t participantID, uint32_t seasonID)
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

std::string HandleWriteGetSeasonChallenges(uint32_t requestID, uint32_t seasonID)
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

void HandleReadGetChallenges(json response, uint32_t participantID, uint32_t seasonID)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        json challenges = response.at("Data");
        for (const json& challenge : challenges)
        {
            uint32_t challengeID = challenge.at("SeasonChallengeId").get<uint32_t>();
            if (!spChallengeManager->SetChallengeStatus(challengeID, CHALLENGE_STATUS_COMPLETE))
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

void HandleReadGetSeasonChallenges(json response, uint32_t seasonID)
{
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        json challenges = response.at("Data");
        const uint32_t hidden_category = spChallengeManager->GetChallengeCategory("Tiebreakers");
        for (const json& challenge : challenges)
        {
            uint32_t challengeID = challenge.at("SeasonChallengeId").get<uint32_t>();
            if (!spChallengeManager->GetChallenge(challengeID))
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

                std::string categoryName = challenge.at("CategoryName").get<std::string>();
                newChallenge._category = spChallengeManager->GetChallengeCategory(categoryName);

                if (newChallenge._category == 0)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Challenge \"%\" (ID: %) has unknown category: %", newChallenge._name, challengeID, categoryName);
                }
                else if (newChallenge._category == hidden_category)
                {
                    newChallenge._status = CHALLENGE_STATUS_HIDDEN;
                }

                std::string difficultyNames = challenge.at("Difficulties").get<std::string>();
                for (const auto& difficultyEntry : challengeDifficultyMap)
                {
                    if (difficultyNames.find(difficultyEntry.first) != std::string::npos)
                    {
                        newChallenge._difficultyBitMask |= difficultyEntry.second;
                    }
                }
                if (newChallenge._difficultyBitMask == CHALLENGE_DIFFICULTY_UNDEFINED)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Challenge \"%\" (ID: %) has unknown difficulty: %", newChallenge._name, challengeID, difficultyNames);
                }

                spChallengeManager->AddChallenge(newChallenge);
            }
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get season challenges: %", response.at("ErrorMessage"));
    }
}