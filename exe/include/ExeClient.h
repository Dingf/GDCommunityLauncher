#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include "Client.h"
#include "Version.h"

class ExeClient : public Client
{
    public:
        static ExeClient* GetInstance();

        bool HasSeasons() const { return !_seasons.empty(); }

        const std::string& GetRole() const { return _role; }

        const std::string& GetLauncherURL() const { return _launcherURL; }
        const std::unordered_map<std::wstring, std::string>& GetDownloadList() const { return _downloadList; }

        void SetLauncherURL(const std::string& launcherURL) { _launcherURL = launcherURL; }
        void SetRole(const std::string& role) { _role = role; }
        void SetUsername(const std::string& username) { _username = username; }
        void SetPassword(const std::string& password) { _password = password; }
        void SetAuthToken(const std::string& authToken) { _authToken = authToken; }
        void SetRefreshToken(const std::string& refreshToken) { _refreshToken = refreshToken; }
        void SetSeasonName(const std::string& seasonName) { _seasonName = seasonName; }
        void SetHostName(const std::string& host) { _host = host; }
        void SetChatURL(const std::string& url) { _chatURL = url; }
        void SetBranch(SeasonBranch branch) { _branch = branch; }

        void AddSeason(const SeasonInfo& seasonInfo) { _seasons.push_back(seasonInfo); }

        bool WriteDataToPipe(void* pipe) const;

    private:
        ExeClient() {}
        ExeClient(ExeClient&) = delete;
        void operator=(const ExeClient&) = delete;

        std::string _role;
        std::string _filename;
        std::string _checksum;
        std::string _version;
        std::string _launcherURL;
        std::unordered_map<std::wstring, std::string> _downloadList;
        std::vector<SeasonInfo> _seasons;
};

#define spClient ExeClient::GetInstance()

#endif//INC_GDCL_EXE_CLIENT_H