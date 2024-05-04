#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include <Windows.h>
#include "ClientBase.h"
#include "ServerAuth.h"

class Client : public ClientBase
{
    public:
        static Client& GetInstance();

        bool HasUpdate() const { return _hasUpdate; }

        void SetUsername(const std::string& username) { _username = username; }
        void SetPassword(const std::string& password) { _password = password; }
        void SetSeasonName(const std::string& seasonName) { _seasonName = seasonName; }
        void SetBranch(SeasonBranch branch) { _branch = branch; }
        void SetGameURL(const std::string& url) { _gameURL = url; }
        void SetChatURL(const std::string& url) { _chatURL = url; }
        void SetUpdateFlag(bool update) { _hasUpdate = update; }

        void AddSeasonInfo(const SeasonInfo& seasonInfo) { _seasons.push_back(seasonInfo); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetRole() const { return _role; }

        friend ServerAuthResult ServerAuthenticate(ServerAuthCallback callback);

    private:
        Client() : _hasUpdate(false) {}
        Client(Client&) = delete;
        void operator=(const Client&) = delete;

        bool        _hasUpdate;
        std::string _role;
};

#endif//INC_GDCL_EXE_CLIENT_H