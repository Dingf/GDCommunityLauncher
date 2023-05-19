#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "ClientBase.h"

class Client
{
    public:
        static Client& GetInstance();

        bool Initialize();
        void Cleanup();

        bool IsValid() const { return _data.IsValid(); }

        bool IsOnline() const { return _online; }

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        uint32_t GetParticipantID() const { return _data._participantID; }

        const std::string&  GetVersionInfoText() const { return _versionInfoText; }
        const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const std::string& GetUsername() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetSeasonName() const { return _data._seasonName; }
        const std::string& GetBranch() const { return _data._branch; }

        const URI& GetServerGameURL() const { return _data._gameURL; }
        const URI& GetServerChatURL() const { return _data._chatURL; }

        const std::vector<SeasonInfo>& GetSeasons() const { return _data._seasons; }
        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }

        std::wstring GetActiveCharacterName() const { return _activeCharacter._name; }
        std::string GetActiveModName() const { if (IsInActiveSeason()) return _activeSeason->_modName; else return {}; }

        bool IsInActiveSeason() const { return (_activeSeason != nullptr) && (!_activeSeason->_modName.empty()); }
        bool IsParticipatingInSeason() const { return IsInActiveSeason() && (_activeCharacter._hasToken); }

        void SetActiveSeason(const std::string& modName, bool hardcore);
        void SetActiveCharacter(const std::wstring& name, bool hasToken);

        void SetParticipantID(uint32_t participantID) { _data._participantID = participantID; }

        void UpdateSeasonStanding();

        static bool UpdateRefreshToken();
        static bool UpdateConnectionStatus();

    private:
        Client() : _activeSeason(nullptr), _online(false) {}
        Client(Client&) = delete;
        void operator=(const Client&) = delete;

        void UpdateVersionInfoText();
        void UpdateLeagueInfoText();

        void CreatePlayMenu();

        void ReadDataFromPipe();

        bool _online;

        uint32_t _rank;
        uint32_t _points;
        
        ClientData _data;
        SeasonInfo* _activeSeason;
        CharacterInfo _activeCharacter;

        std::string _versionInfoText;
        std::wstring _leagueInfoText;
};

#endif//INC_GDCL_DLL_CLIENT_H