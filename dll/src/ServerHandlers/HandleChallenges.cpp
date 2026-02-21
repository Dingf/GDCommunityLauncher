#include <string>
#include "ChallengeManager.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetChallenges(uint32_t participantID, uint32_t seasonID)
{
    json request = 
    {
        { "RequestName", "GetParticipantChallenges" },
        { "Arguments", {
            { "SeasonId", seasonID },
            { "SeasonParticipantId", participantID }
        }}
    };
    return request.dump();
}

void HandleReadGetChallenges(json response)
{
    // TODO: This is actually the wrong endpoint; this one should be for the completed challenges. Make sure to move when there is a new endpoint for all challenges
    std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        json challenges = response.at("Data");
        for (json challenge : challenges)
        {
            uint32_t challengeID = challenge.at("SeasonChallengeId").get<uint32_t>();
            if (!spChallengeManager->GetChallenge(challengeID))
            {
                Challenge newChallenge;
                newChallenge._name = challenge.at("ChallengeName").get<std::string>();
                newChallenge._challengeID = challengeID;
                newChallenge._maxLevel = challenge.at("MaxLevel").get<uint32_t>();
                newChallenge._status = CHALLENGE_STATUS_INCOMPLETE;
                newChallenge._points = challenge.at("PointValue").get<uint32_t>();

                std::string categoryName = challenge.at("CategoryName").get<std::string>();
                newChallenge._category = spChallengeManager->GetChallengeCategory(categoryName);

                if (newChallenge._category == 0)
                    Logger::LogMessage(LOG_LEVEL_WARN, "Challenge \"%\" (ID: %) has unknown category: %", newChallenge._name, challengeID, categoryName);

                // TODO: Implement the challenge difficulty mask parsing

                spChallengeManager->AddChallenge(newChallenge);
            }
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to get participant challenges: %", response.at("ErrorMessage"));
    }
}