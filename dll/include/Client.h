#ifndef INC_GDCL_DLL_CLIENT_H
#define INC_GDCL_DLL_CLIENT_H

#include <stdint.h>
#include <string>

class Client
{
    public:
        static Client& GetInstance();

        bool SetupClientHooks();
        void CleanupClientHooks();

        bool IsValid() const { return (!_name.empty() && !_authToken.empty()); }

        uint32_t GetPoints() const { return _points; }
        uint32_t GetRank() const { return _rank; }

        const std::string&  GetVersionInfoText() const { return _versionInfoText; }
        const std::wstring& GetLeagueInfoText()  const { return _leagueInfoText; }

        const std::string& GetName() const { return _name; }
        const std::string& GetAuthToken() const { return _authToken; }
        const std::string& GetHostName() const { return _hostName; }

        //TODO: These functions are only for testing. Delete them later when the web API is implemented
        void SetPoints(uint32_t points)
        {
            _points = points;
            UpdateLeagueInfoText();
        }
        void SetRank(uint32_t rank)
        {
            _rank = rank;
            UpdateLeagueInfoText();
        }

    private:
        Client() : _name({}), _authToken({}), _rank(0), _points(0) {}

        void UpdateVersionInfoText();
        void UpdateLeagueInfoText();

        void ReadDataFromPipe();

        uint32_t _rank;
        uint32_t _points;

        std::string _name;
        std::string _authToken;
        std::string _hostName;
        std::string _versionInfoText;
        std::wstring _leagueInfoText;
};

#endif//INC_GDCL_DLL_CLIENT_H