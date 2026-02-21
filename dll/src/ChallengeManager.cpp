#include "ChallengeManager.h"

const std::unordered_map<std::string, uint32_t> challengeCategoryMap =
{
    { "Level Limited Challenges", 1 },
    { "Shattered Realm Clears", 2 },
    { "Skeleton Key Dungeons", 3 },
    { "World Bosses", 4 },
    { "Super Bosses", 5 },
    { "Nemesis Bosses", 6 },
    { "Campaign/Story", 7 },
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

void ChallengeManager::SetChallengeStatus(uint32_t challengeID, ChallengeStatus status)
{
    auto it = _challengeList.find(challengeID);
    if (it != _challengeList.end())
        it->second->_status = status;
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