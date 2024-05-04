#ifndef INC_GDCL_CLIENT_BASE_H
#define INC_GDCL_CLIENT_BASE_H

#include <string>
#include <vector>
#include "URI.h"

enum SeasonType
{
    SEASON_TYPE_NONE = 0,
    SEASON_TYPE_SC_TRADE = 1,
    SEASON_TYPE_HC_SSF = 2,
};

enum SeasonBranch
{
    SEASON_BRANCH_OFFLINE = 0,
    SEASON_BRANCH_RELEASE = 1,
    SEASON_BRANCH_BETA = 2,
};

struct SeasonInfo
{
    uint32_t    _seasonID;
    uint32_t    _seasonType;
    std::string _displayName;
    std::string _participationToken;
};

class ClientBase
{
    public:
        bool IsInitialized() const
        {
            return (!_username.empty() && !_authToken.empty() && !_refreshToken.empty());
        }

        bool IsOfflineMode() const { return _branch == SEASON_BRANCH_OFFLINE; }
        bool HasSeasons() const { return !_seasons.empty(); }

        const std::string& GetUsername() const { return _username; }
        const std::string& GetAuthToken() const { return _authToken; }
        const std::string& GetRefreshToken() const { return _refreshToken; }
        const std::string& GetSeasonName() const { return _seasonName; }
        SeasonBranch GetBranch() const { return _branch; }

        std::string GetBranchName() const
        {
            switch (_branch)
            {
                case SEASON_BRANCH_OFFLINE:
                    return "offline";
                case SEASON_BRANCH_RELEASE:
                    return "prod";
                case SEASON_BRANCH_BETA:
                    return "beta";
                default:
                    return "";
            }
        }

        const URI& GetServerGameURL() const { return _gameURL; }
        const URI& GetServerChatURL() const { return _chatURL; }

        const std::vector<SeasonInfo>& GetSeasonList() const { return _seasons; }

    protected:
        ClientBase() : _branch(SEASON_BRANCH_RELEASE) {}

        std::string  _username;
        std::string  _password;
        std::string  _authToken;
        std::string  _refreshToken;
        std::string  _seasonName;
        SeasonBranch _branch;
        URI          _gameURL;
        URI          _chatURL;
        std::vector<SeasonInfo> _seasons;
};

#endif//INC_GDCL_CLIENT_BASE_H