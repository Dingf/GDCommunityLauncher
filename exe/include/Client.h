#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <Windows.h>

class Client
{
    public:
        static Client& GetInstance(const std::string& name = {}, const std::string& authToken = {}, const std::string& hostName = {});

        bool IsValid() const { return (!_name.empty() && !_authToken.empty()); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetName() const { return _name; }
        const std::string& GetAuthToken() const { return _authToken; }
        const std::string& GetHostName() const { return _hostName; }

    private:
        Client(const std::string& name, const std::string& authToken, const std::string& hostName) : _name(name), _authToken(authToken), _hostName(hostName) {}

        std::string _name;
        std::string _authToken;
        std::string _hostName;
};

#endif//INC_GDCL_DLL_CLIENT_H