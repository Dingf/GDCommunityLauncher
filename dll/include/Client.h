#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>

enum SeasonType
{
    SEASON_TYPE_NONE = 0,
    SEASON_TYPE_SC_TRADE = 1,
    SEASON_TYPE_HC_SSF = 2,
    NUM_SEASON_TYPE = 3,
};

struct SeasonInfo
{
    uint32_t    _seasonID;
    uint32_t    _seasonType;
    std::string _modName;
    std::string _displayName;
    std::string _participationToken;
};

struct CharacterInfo
{
    bool         _hasToken;
    std::wstring _name;
};

struct ClientData
{
    bool IsValid() const
    {
        return (!_username.empty() && !_authToken.empty() && !_refreshToken.empty() && !_hostName.empty());
    }

    uint32_t    _participantID;
    std::string _username;
    std::string _authToken;
    std::string _refreshToken;
    std::string _hostName;
    std::vector<SeasonInfo> _seasons;
};

class Client
{
    public:
        static Client& GetInstance();

        bool SetupClientHooks();
        void CleanupClientHooks();

        void PostRefreshToken();

        bool IsValid() const { return _data.IsValid(); }

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        uint32_t GetParticipantID() const { return _data._participantID; }

        const std::string&  GetVersionInfoText() const { return _versionInfoText; }
        const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const std::string& GetUsername() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }

        const std::vector<SeasonInfo>& GetSeasons() const { return _data._seasons; }
        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }

        const std::wstring& GetActiveCharacterName() const { return _activeCharacter._name; }

        bool IsParticipatingInSeason() const { return (_activeSeason != nullptr) && (_activeCharacter._hasToken); }

        void SetActiveSeason(const std::string& modName, bool hardcore);
        void SetActiveCharacter(const std::wstring& name, bool hasToken);

        void SetParticipantID(uint32_t participantID)
        {
            _data._participantID = participantID;
        }
        void SetPoints(uint32_t points)
        {
            _points = points;
            UpdateLeagueInfoText();
        }
        void SetRank(uint32_t rank)
        {
            _rank = rank;
            UpdateLeagueInfoText();
        }

    private:
        Client() { _activeSeason = nullptr; }

        void UpdateVersionInfoText();
        void UpdateLeagueInfoText();

        void ReadDataFromPipe();

        uint32_t _rank;
        uint32_t _points;
        
        ClientData _data;
        SeasonInfo* _activeSeason;
        CharacterInfo _activeCharacter;

        std::string _versionInfoText;
        std::wstring _leagueInfoText;
};

#endif//INC_GDCL_DLL_CLIENT_H