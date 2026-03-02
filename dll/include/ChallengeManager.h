#ifndef INC_GDCL_DLL_CHALLENGE_MANAGER_H
#define INC_GDCL_DLL_CHALLENGE_MANAGER_H

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
    bool                _active;
};

class ChallengeManager
{
    public:
        typedef std::unordered_map<uint32_t, std::unique_ptr<Challenge>> ChallengeList;

        ChallengeManager(ChallengeManager&) = delete;
        void operator=(const ChallengeManager&) = delete;

        static ChallengeManager* GetInstance();

        uint32_t GetChallengeCategory(std::string categoryName) const;

        const Challenge* GetChallenge(uint32_t challengeID) const;
        const ChallengeList& GetChallengeList() const { return _challengeList; }

        bool SetChallengeStatus(uint32_t challengeID, ChallengeStatus status);

        void AddChallenge(const Challenge& challenge);

    private:
        ChallengeManager() = default;

        ChallengeList _challengeList;
};

#define spChallengeManager ChallengeManager::GetInstance()

#endif//INC_GDCL_DLL_CHALLENGE_MANAGER_H