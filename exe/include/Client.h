#ifndef INC_GDCL_EXE_CLIENT_H
#define INC_GDCL_EXE_CLIENT_H

#include <string>
#include <Windows.h>

class Client
{
    public:
        static Client& GetInstance(const std::string& name = {}, const std::string& authToken = {});

        bool IsValid() const { return (!_name.empty() && !_authToken.empty()); }

        bool WriteDataToPipe(HANDLE pipe) const;

        const std::string& GetName() const { return _name; }
        const std::string& GetAuthToken() const { return _authToken; }

    private:
        Client(const std::string& name, const std::string& authToken) : _name(name), _authToken(authToken) {}

        std::string _name;
        std::string _authToken;
};

#endif//INC_GDCL_DLL_CLIENT_H