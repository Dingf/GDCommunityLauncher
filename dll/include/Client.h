#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>

enum SeasonType
{
    SEASON_TYPE_SC_TRADE = 1,
    SEASON_TYPE_HC_SSF = 2,
};

class Client
{
    public:
        static Client& GetInstance();

        bool SetupClientHooks();
        void CleanupClientHooks();

        void SendRefreshToken();

        bool IsValid() const { return _data.IsValid(); }

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }
        uint32_t GetParticipantID() const { return _data._participantID; }

        const std::string&  GetVersionInfoText() const { return _versionInfoText; }
        const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const std::string& GetName() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }
        const std::string& GetSeasonName() const { return _data._seasonName; }
        const std::string& GetSeasonModName() const { return _data._seasonModName; }

        //TODO: These functions are only for testing. Delete them later when the web API is implemented
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
        Client() {}

        void UpdateVersionInfoText();
        void UpdateLeagueInfoText();

        void ReadDataFromPipe();

        uint32_t _rank;
        uint32_t _points;

        struct ServerData
        {
            bool IsValid() const
            {
                return (!_username.empty() && !_authToken.empty() && !_refreshToken.empty() && !_hostName.empty() && !_seasonName.empty() && !_seasonModName.empty());
            }

            uint32_t    _participantID;
            std::string _username;
            std::string _authToken;
            std::string _refreshToken;
            std::string _hostName;
            std::string _seasonName;
            std::string _seasonModName;
        } _data;

        std::string _versionInfoText;
        std::wstring _leagueInfoText;
};

#endif//INC_GDCL_DLL_CLIENT_H