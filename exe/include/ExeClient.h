#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include "Client.h"
#include "Version.h"

enum SeasonRegion
{
    SEASON_REGION_US_SE = 0,
    SEASON_REGION_BR = 1,
    SEASON_REGION_DE = 2,
    SEASON_REGION_FR = 3,
    SEASON_REGION_ID = 4,
    SEASON_REGION_NL = 5,
    SEASON_REGION_SG = 6,
};

class ExeClient : public Client
{
    public:
        static ExeClient* GetInstance();

        bool HasSeasons() const { return !_seasons.empty(); }

        const SeasonRegion& GetRegion() const { return _region; }
        std::string GetRegionName() const
        {
            switch (_region)
            {
                case SEASON_REGION_BR:
                    return "br";
                case SEASON_REGION_DE:
                    return "de";
                case SEASON_REGION_FR:
                    return "fr";
                case SEASON_REGION_ID:
                    return "id";
                case SEASON_REGION_NL:
                    return "nl";
                case SEASON_REGION_SG:
                    return "sg";
                default:
                    return "us-southeast-1";
            }
        }

        const std::string& GetLauncherURL() const { return _launcherURL; }
        const std::unordered_map<std::wstring, std::string>& GetDownloadList() const { return _downloadList; }

        void SetLauncherURL(const std::string& launcherURL) { _launcherURL = launcherURL; }
        void SetUsername(const std::string& username) { _username = username; }
        void SetPassword(const std::string& password) { _password = password; }
        void SetAuthToken(const std::string& authToken) { _authToken = authToken; }
        void SetRefreshToken(const std::string& refreshToken) { _refreshToken = refreshToken; }
        void SetSeasonName(const std::string& seasonName) { _seasonName = seasonName; }
        void SetHostName(const std::string& host) { _host = host; }
        void SetChatURL(const std::string& url) { _chatURL = url; }
        void SetBranch(SeasonBranch branch) { _branch = branch; }
        void SetRegion(const SeasonRegion& region) { _region = region; }

        void AddSeason(const SeasonInfo& seasonInfo) { _seasons.push_back(seasonInfo); }

        bool WriteDataToPipe(void* pipe) const;

    private:
        ExeClient() {}
        ExeClient(ExeClient&) = delete;
        void operator=(const ExeClient&) = delete;

        std::string _filename;
        std::string _checksum;
        std::string _version;
        std::string _launcherURL;
        SeasonRegion _region;
        std::unordered_map<std::wstring, std::string> _downloadList;
        std::vector<SeasonInfo> _seasons;
};

#define spClient ExeClient::GetInstance()

#endif//INC_GDCL_EXE_CLIENT_H