#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <vector>
#include <Windows.h>
#include "ClientBase.h"

class Client
{
    public:
        static Client& GetInstance(const ClientData& serverData = {});

        Client& operator=(const ClientData& data)
        {
            _data._username = data._username;
            _data._authToken = data._authToken;
            _data._refreshToken = data._refreshToken;
            _data._hostName = data._hostName;
            _data._updateFlag = data._updateFlag;
            _data._seasons = data._seasons;
            return *this;
        }

        bool IsValid() const { return _data.IsValid(); }
        bool HasUpdate() const { return _data._updateFlag; }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetUsername() const { return _data._username; }
        const std::string& GetRefreshToken() const { return _data._refreshToken; }
        const std::string& GetAuthToken() const { return _data._authToken; }
        const std::string& GetHostName() const { return _data._hostName; }
        const std::string& GetRole() const { return _data._role; }

    private:
        Client(const ClientData& data) { *this = data; }

        ClientData _data;
};

#endif//INC_GDCL_EXE_CLIENT_H