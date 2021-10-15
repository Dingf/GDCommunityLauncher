#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include <Windows.h>

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

struct ClientData
{
    bool IsValid() const
    {
        return (!_username.empty() && !_authToken.empty() && !_refreshToken.empty() && !_hostName.empty());
    }

    std::string _username;
    std::string _authToken;
    std::string _refreshToken;
    std::string _hostName;
    std::vector<SeasonInfo> _seasons;
};

class Client
{
    public:
        static Client& GetInstance(const ClientData& serverData = {});

        bool IsValid() const { return _data.IsValid(); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetUsername() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }

    private:
        Client(const ClientData& data)
        {
            _data._username = data._username;
            _data._authToken = data._authToken;
            _data._refreshToken = data._refreshToken;
            _data._hostName = data._hostName;
            _data._seasons = data._seasons;
        }

        ClientData _data;
};

#endif//INC_GDCL_DLL_CLIENT_H