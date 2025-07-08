#ifndef INC_GDCL_DLL_SEASON_CLIENT_H
#define INC_GDCL_DLL_SEASON_CLIENT_H

#include <stdint.h>
#include <string>
#include "Client.h"

class SeasonClient : public Client
{
    public:
        SeasonClient(SeasonClient&) = delete;
        void operator=(const SeasonClient&) = delete;

        static SeasonClient* GetInstance();

        static bool Initialize();

        // TODO: Clean up this and the other functions below
        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        //uint32_t GetParticipantID() const { return _participantID; }

        //const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }
        //std::wstring GetActiveCharacter() const { return _activeCharacter; }

        //Connection* GetConnection() { return _connection.get(); }

        bool IsInActiveSeason() const { return _activeSeason != nullptr; }
        bool IsPlayingSeasonOnline() const { return IsInActiveSeason() && !_activeCharacter.empty(); }
        bool IsPlayingSeason() const { return IsOfflineMode() || IsPlayingSeasonOnline(); }

        void SetActiveSeason(bool hardcore);
        void SetActiveCharacter(const std::wstring& character) { _activeCharacter = character; }

        //void SetParticipantID(uint32_t participantID);
        //void UpdateSeasonStanding();

    private:
        SeasonClient();

        void ReadDataFromPipe();

        //static int64_t UpdateRefreshToken();
        //static int64_t UpdateConnectionStatus();

        //static void OnRefreshToken(const signalr::value& value, const std::vector<void*> args);
        //static void OnUpdateSeasonStanding(const signalr::value& value, const std::vector<void*> args);

        uint32_t _rank;
        uint32_t _points;
        //uint32_t _participantID;
        std::wstring _activeCharacter;
        const SeasonInfo* _activeSeason;

        //std::unique_ptr<Connection> _connection;
};

#define spClient SeasonClient::GetInstance()

#endif//INC_GDCL_DLL_GAME_CLIENT_H