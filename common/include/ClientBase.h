#ifndef INC_GDCL_CLIENT_BASE_H
#define INC_GDCL_CLIENT_BASE_H

#include <string>
#include <vector>

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

struct CharacterInfo
{
    bool         _hasToken;     // GameAPI::HasToken() only works for the current difficulty; this stores whether the token exists on any difficulty (usually Normal)
    std::wstring _name;
};

struct ClientData
{
    bool IsValid() const
    {
        return (!_username.empty() && !_authToken.empty() && !_refreshToken.empty() && !_hostName.empty());
    }

    bool        _updateFlag;
    uint32_t    _participantID;
    std::string _role;
    std::string _username;
    std::string _authToken;
    std::string _refreshToken;
    std::string _hostName;
    std::vector<SeasonInfo> _seasons;
};

#endif//INC_GDCL_CLIENT_BASE_H