#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>

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

        const std::string& GetName() const { return _data._name; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }
        const std::string& GetLeagueName() const { return _data._leagueName; }
        const std::string& GetLeagueModName() const { return _data._leagueModName; }

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
                return (!_name.empty() && !_authToken.empty() && !_refreshToken.empty() && !_hostName.empty() && !_leagueName.empty() && !_leagueModName.empty());
            }

            std::string _name;
            std::string _authToken;
            std::string _refreshToken;
            std::string _hostName;
            std::string _leagueName;
            std::string _leagueModName;
            uint32_t    _participantID;
        } _data;

        std::string _versionInfoText;
        std::wstring _leagueInfoText;
};

#endif//INC_GDCL_DLL_CLIENT_H