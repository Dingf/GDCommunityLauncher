#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include <Windows.h>

enum SeasonType
{
    SEASON_TYPE_SC_TRADE = 1,
    SEASON_TYPE_HC_SSF = 2,
};

class Client
{
    public:
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
        };

        static Client& GetInstance(const ServerData& data = {});

        bool IsValid() const { return _data.IsValid(); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetUsername() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }
        const std::string& GetSeasonName() const { return _data._seasonName; }
        const std::string& GetSeasonModName() const { return _data._seasonModName; }

    private:
        Client(const ServerData& data)
        {
            _data._username = data._username;
            _data._authToken = data._authToken;
            _data._refreshToken = data._refreshToken;
            _data._hostName = data._hostName;
            _data._seasonName = data._seasonName;
            _data._seasonModName = data._seasonModName;
        }

        ServerData _data;
};

#endif//INC_GDCL_DLL_CLIENT_H