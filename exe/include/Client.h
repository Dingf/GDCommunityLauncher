#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <Windows.h>

class Client
{
    public:
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
        };

        static Client& GetInstance(const ServerData& data = {});

        bool IsValid() const { return _data.IsValid(); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetName() const { return _data._name; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }
        const std::string& GetLeagueName() const { return _data._leagueName; }
        const std::string& GetLeagueModName() const { return _data._leagueModName; }

    private:
        Client(const ServerData& data)
        {
            _data._name = data._name;
            _data._authToken = data._authToken;
            _data._refreshToken = data._refreshToken;
            _data._hostName = data._hostName;
            _data._leagueName = data._leagueName;
            _data._leagueModName = data._leagueModName;
        }

        ServerData _data;
};

#endif//INC_GDCL_DLL_CLIENT_H