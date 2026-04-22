#include "ChallengeManager.h"
#include "GameAPI/Difficulty.h"

const std::unordered_map<std::string, uint32_t> challengeDifficultyMap =
{
    { "Normal", (1 << GameAPI::GAME_DIFFICULTY_NORMAL) },
    { "Elite", (1 << GameAPI::GAME_DIFFICULTY_ELITE) },
    { "Ultimate", (1 << GameAPI::GAME_DIFFICULTY_ULTIMATE) }
};

ChallengeManager* ChallengeManager::GetInstance()
{
    static ChallengeManager instance;
    return &instance;
}

const std::map<uint32_t, std::string>& ChallengeManager::GetChallengeCategories() const
{
    static const std::map<uint32_t, std::string> challengeCategoryMap =
    {
        { 1, "Skeleton Key Dungeons" },
        { 2, "Shattered Realm Clears" },
        { 3, "World Bosses" },
        { 4, "Nemesis Bosses" },
        // No 5 or 6 in any challenge
        { 7, "Leveling Achievements" },
        { 8, "Super Bosses" },
        { 9, "Campaign/Story" },
        { 10, "Tiebreakers" },
        { 11, "Level Limited Challenges" },
    };

    return challengeCategoryMap;
}

const std::unordered_map<std::string, uint32_t>& ChallengeManager::GetChallengeDifficulties() const
{
    return challengeDifficultyMap;
}

const Challenge* ChallengeManager::GetChallenge(uint32_t seasonID, uint32_t challengeID) const
{
    if (const ChallengeList* challengeList = GetChallengeList(seasonID))
    {
        auto it = challengeList->find(challengeID);
        if (it != challengeList->end())
            return it->second.get();
    }
    return nullptr;
}

const ChallengeManager::ChallengeList* ChallengeManager::GetChallengeList(uint32_t seasonID) const
{
    auto it = _challengeList.find(seasonID);
    return (it != _challengeList.end()) ? &it->second : nullptr;
}

bool ChallengeManager::SetChallengeStatus(uint32_t seasonID, uint32_t challengeID, ChallengeStatus status)
{
    if (const ChallengeList* challengeList = GetChallengeList(seasonID))
    {
        auto it = challengeList->find(challengeID);
        if (it != challengeList->end())
        {
            it->second->_status = status;
            return true;
        }
    }
    return false;
}

void ChallengeManager::AddChallenge(uint32_t seasonID, const Challenge& challenge)
{
    uint32_t challengeID = challenge._challengeID;
    _challengeList[seasonID][challengeID] = std::make_unique<Challenge>(challenge);
}