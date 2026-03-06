#ifndef INC_GDCL_CLIENT_H
#define INC_GDCL_CLIENT_H

#include <string>
#include <vector>
#include "URI.h"

enum SeasonBranch
{
    SEASON_BRANCH_OFFLINE = 0,
    SEASON_BRANCH_RELEASE = 1,
    SEASON_BRANCH_BETA = 2,
};

enum SeasonType : uint32_t
{
    SEASON_TYPE_NONE = 0,
    SEASON_TYPE_SC_TRADE = 1,
    SEASON_TYPE_HC_SSF = 2,
};

struct SeasonInfo
{
    uint32_t    _seasonID;
    SeasonType  _seasonType;
    std::string _modName;
    std::string _displayName;
    std::string _participationToken;
};

constexpr char OFFLINE_SEASON_NAME[] = "GrimLeagueS07";

class Client
{
    public:

        bool IsOfflineMode() const { return _branch == SEASON_BRANCH_OFFLINE; }

        const std::string& GetUsername() const { return _username; }
        const std::string& GetPassword() const { return _password; }
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

        const URI& GetHostName() const { return _host; }
        const URI& GetServerChatURL() const { return _chatURL; }    // TODO: This may be obsolete if chat gets merged into the server websocket; delete if so

    protected:
        Client() : _branch(SEASON_BRANCH_RELEASE) {}

        std::string  _username;
        std::string  _password;
        std::string  _authToken;
        std::string  _refreshToken;
        std::string  _seasonName;
        SeasonBranch _branch;
        URI          _host;
        URI          _chatURL;
};

#endif//INC_GDCL_CLIENT_BASE_H