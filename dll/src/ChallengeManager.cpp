#include "ChallengeManager.h"

const std::unordered_map<std::string, uint32_t> challengeCategoryMap =
{
    { "Skeleton Key Dungeons", 1 },
    { "Shattered Realm Clears", 2 },
    { "World Bosses", 3 },
    { "Nemesis Bosses", 4 },
    // No 5 or 6 in any challenge
    { "Leveling Achievements", 7 },
    { "Super Bosses", 8 },
    { "Campaign/Story", 9 },
    { "Tiebreakers", 10 },
    { "Level Limited Challenges", 11 },
};

ChallengeManager* ChallengeManager::GetInstance()
{
    static ChallengeManager instance;
    return &instance;
}

uint32_t ChallengeManager::GetChallengeCategory(std::string categoryName) const
{
    auto it = challengeCategoryMap.find(categoryName);
    return (it != challengeCategoryMap.end()) ? it->second : 0;
}

const Challenge* ChallengeManager::GetChallenge(uint32_t challengeID) const
{
    auto it = _challengeList.find(challengeID);
    return (it != _challengeList.end()) ? it->second.get() : nullptr;
}

bool ChallengeManager::SetChallengeStatus(uint32_t challengeID, ChallengeStatus status)
{
    auto it = _challengeList.find(challengeID);
    if (it != _challengeList.end())
    {
        it->second->_status = status;
        return true;
    }
    return false;
}

void ChallengeManager::AddChallenge(const Challenge& challenge)
{
    uint32_t challengeID = challenge._challengeID;
    auto it = _challengeList.find(challengeID);
    if (it != _challengeList.end())
    {
        *(it->second) = challenge;
    }
    else
    {
        _challengeList[challengeID] = std::make_unique<Challenge>(challenge);
    }
}