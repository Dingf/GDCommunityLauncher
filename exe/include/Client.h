#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <Windows.h>
#include "ClientBase.h"
#include "ServerAuth.h"
#include "Connection.h"
#include "Version.h"

constexpr char OFFLINE_SEASON_NAME[] = "GrimLeagueS07";

class Client : public ClientBase
{
    public:
        struct LauncherData
        {
            LauncherData() : _hasUpdate(false), _size(0) {}

            bool        _hasUpdate;
            size_t      _size;
            std::string _filename;
            std::string _checksum;
            std::string _version;
            std::string _downloadURL;
        };

        static Client& GetInstance();

        bool HasLauncherUpdate() const { return _launcher._hasUpdate; }
        const std::string& GetRole() const { return _role; }

        const std::unordered_map<std::wstring, std::string>& GetDownloadList() const { return _downloadList; }

        Connection* GetConnection() { return _connection.get(); }

        void SetUsername(const std::string& username) { _username = username; }
        void SetPassword(const std::string& password) { _password = password; }
        void SetBranch(SeasonBranch branch) { _branch = branch; }
        void SetSeasonName(const std::string& seasonName) { _seasonName = seasonName; }
        void CreateConnection(const std::string& url);

        bool WriteDataToPipe(HANDLE pipe) const;

        static void OnLogin(const signalr::value& value);
        static void OnGetChatUrl(const signalr::value& value);
        static void OnGetSeasonName(const signalr::value& value);
        static void OnGetSeasonData(const signalr::value& value);
        static void OnGetLauncherVersion(const signalr::value& value);
        static void OnGetSeasonFiles(const signalr::value& value);

    private:
        Client() {}
        Client(Client&) = delete;
        void operator=(const Client&) = delete;

        std::unique_ptr<Connection> _connection;

        std::string  _role;
        LauncherData _launcher;
        std::unordered_map<std::wstring, std::string> _downloadList;
};

#endif//INC_GDCL_EXE_CLIENT_H