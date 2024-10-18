#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <memory>
#include <unordered_map>
#include "ClientBase.h"
#include "Connection.h"

class Client : public ClientBase
{
    public:
        static Client& GetInstance();

        bool Initialize();

        bool IsOnline() const { return _online; }

        void SetBranch(SeasonBranch branch) {}

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        uint32_t GetCurrentParticipantID() const { return _participantID; }

        const std::string&  GetVersionInfoText() const { return _versionInfoText; }
        const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }
        std::wstring GetActiveCharacter() const { return _activeCharacter; }

        Connection* GetConnection() { return _connection.get(); }

        bool IsInActiveSeason() const { return _activeSeason != nullptr; }
        bool IsPlayingSeasonOnline() const { return IsInActiveSeason() && !_activeCharacter.empty(); }
        bool IsPlayingSeason() const { return IsOfflineMode() || IsPlayingSeasonOnline(); }

        void SetActiveSeason(bool hardcore);
        void SetActiveCharacter(const std::wstring& character) { _activeCharacter = character; }

        void SetParticipantID(uint32_t participantID);
        void UpdateSeasonStanding();

    private:
        Client();
        Client(Client&) = delete;
        void operator=(const Client&) = delete;

        void ReadDataFromPipe();

        static bool UpdateRefreshToken();
        static bool UpdateConnectionStatus();

        static void OnRefreshToken(const signalr::value& value, const std::vector<void*> args);
        static void OnUpdateSeasonStanding(const signalr::value& value, const std::vector<void*> args);

        void UpdateLeagueInfoText();
        void UpdateVersionInfoText();

        bool _online;

        uint32_t _rank;
        uint32_t _points;
        uint32_t _participantID;
        
        SeasonInfo*  _activeSeason;
        std::wstring _activeCharacter;
        std::string  _versionInfoText;
        std::wstring _leagueInfoText;

        std::unique_ptr<Connection> _connection;
};

#endif//INC_GDCL_DLL_CLIENT_H