#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "UpdateThread.h"
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
        const std::string& GetHostName() const { return _data._hostName; }

        const std::vector<SeasonInfo>& GetSeasons() const { return _data._seasons; }
        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }

        std::mutex& GetTransferMutex() { return _transferMutex; }

        const std::wstring& GetActiveCharacterName() const { return _activeCharacter._name; }

        bool IsParticipatingInSeason() const { return (_activeSeason != nullptr) && (_activeCharacter._hasToken); }

        void SetActiveSeason(const std::string& modName, bool hardcore);
        void SetActiveCharacter(const std::wstring& name, bool hasToken);

        void SetParticipantID(uint32_t participantID) { _data._participantID = participantID; }

        void UpdateSeasonStanding();

    private:
        Client() : _activeSeason(nullptr), _online(false) {}

        static void UpdateRefreshToken();
        static void UpdateConnectionStatus();
        void UpdateVersionInfoText();
        void UpdateLeagueInfoText();

        void ReadDataFromPipe();

        bool _online;

        uint32_t _rank;
        uint32_t _points;
        
        ClientData _data;
        SeasonInfo* _activeSeason;
        CharacterInfo _activeCharacter;

        std::string _versionInfoText;
        std::wstring _leagueInfoText;
        std::mutex _transferMutex;
};

#endif//INC_GDCL_DLL_CLIENT_H