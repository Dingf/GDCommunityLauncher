#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include "Client.h"
#include "JSON.h"

class DllClient : public Client
{
    public:
        DllClient(DllClient&) = delete;
        void operator=(const DllClient&) = delete;

        static DllClient* GetInstance();

        bool HasSeasons() const { return !_seasons.empty(); }

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        std::wstring GetMainPlayerName() const { return _lastPlayerName; }

        void SetPoints(uint32_t points) { _points = points; }
        void SetRank(uint32_t rank) { _rank = rank; }
        void SetMainPlayerName(const std::wstring& characterName) { _lastPlayerName = characterName; }

        const SeasonInfo* GetActiveSeason() const { return _activeSeason; }
        const SeasonInfo* GetSeasonByType(SeasonType type);
        const SeasonInfo* GetSeasonByType(bool hardcore);

        bool IsPlayingSeason() const { return _activeSeason != nullptr; }
        bool IsPlayingSeasonOrOffline() const { return IsPlayingSeason() || IsOfflineMode(); }

        void SetActiveSeason(uint32_t seasonID);

              std::vector<SeasonInfo>& GetSeasonList()       { return _seasons; }
        const std::vector<SeasonInfo>& GetSeasonList() const { return _seasons; }

    private:
        DllClient();

        void ReadDataFromPipe();

        uint32_t _rank;
        uint32_t _points;
        std::wstring _lastPlayerName;

        std::vector<SeasonInfo> _seasons;
        const SeasonInfo* _activeSeason;
};

#define spClient DllClient::GetInstance()

#endif//INC_GDCL_DLL_CLIENT_H