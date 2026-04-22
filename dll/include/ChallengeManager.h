#ifndef INC_GDCL_DLL_CHALLENGE_MANAGER_H
#define INC_GDCL_DLL_CHALLENGE_MANAGER_H

#include <map>
#include <string>
#include <unordered_map>
#include <memory>

enum ChallengeStatus : uint32_t
{
    CHALLENGE_STATUS_INCOMPLETE = 0,
    CHALLENGE_STATUS_COMPLETE = 1,
    CHALLENGE_STATUS_HIDDEN = 2,
};

struct Challenge
{
    Challenge() = default;
    std::string         _name;
    uint32_t            _category;
    uint32_t            _challengeID;
    uint32_t            _maxLevel;
    uint32_t            _points;
    ChallengeStatus     _status;
    uint32_t            _difficultyBitMask;
    std::string         _difficultyRaw;
    bool                _active;
};

class ChallengeManager
{
    public:
        typedef std::unordered_map<uint32_t, std::unique_ptr<Challenge>> ChallengeList;

        ChallengeManager(ChallengeManager&) = delete;
        void operator=(const ChallengeManager&) = delete;

        static ChallengeManager* GetInstance();

        const std::map<uint32_t, std::string>& GetChallengeCategories() const;
        const std::unordered_map<std::string, uint32_t>& GetChallengeDifficulties() const;

        const Challenge* GetChallenge(uint32_t seasonID, uint32_t challengeID) const;
        const ChallengeList* GetChallengeList(uint32_t seasonID) const;

        bool SetChallengeStatus(uint32_t seasonID, uint32_t challengeID, ChallengeStatus status);

        void AddChallenge(uint32_t seasonID, const Challenge& challenge);

    private:
        ChallengeManager() = default;

        std::unordered_map<uint32_t, ChallengeList> _challengeList;
};

#define spChallengeManager ChallengeManager::GetInstance()

#endif//INC_GDCL_DLL_CHALLENGE_MANAGER_H