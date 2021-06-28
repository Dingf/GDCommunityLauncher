#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <string>

class Client
{
    public:
        static Client& GetInstance();

        bool SetupClientHooks();
        void CleanupClientHooks();

        bool IsValid() const { return (!_name.empty() && !_authToken.empty()); }

        const std::string& GetName() const { return _name; }
        const std::string& GetAuthToken() const { return _authToken; }

    private:
        Client() : _name({}), _authToken({}) {}

        void ReadDataFromPipe();

        std::string _name;
        std::string _authToken;
};

#endif//INC_GDCL_DLL_CLIENT_H