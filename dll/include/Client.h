#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <string>

class Client
{
    public:
        static Client& GetInstance();

        bool SetupClientHooks();

        void CLeanupClientHooks();

        const std::string& GetName() const { return _name; }
        const std::string& GetAuthToken() const { return _authToken; }

    private:
        Client() : _isInitialized(false) {}

        void ReadClientDataFromPipe();

        bool _isInitialized;
        std::string _name;
        std::string _authToken;
};

#endif