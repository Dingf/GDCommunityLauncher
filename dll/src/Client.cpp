#include <Windows.h>
#include "HookManager.h"
#include "Client.h"
#include "EngineAPI.h"
#include "Log.h"
#include "Version.h"

Client& Client::GetInstance()
{
    static Client instance;
    if (!instance.IsValid())
    {
        instance.ReadDataFromPipe();
    }
    return instance;
}

inline bool ReadStringFromPipe(HANDLE pipe, std::string& s)
{
    DWORD bytesRead;
    uint8_t sizeBuffer[4];
    if (!ReadFile(pipe, &sizeBuffer, 4, &bytesRead, NULL) || (bytesRead != 4))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
        return false;
    }

    uint32_t length = (uint32_t)sizeBuffer[0] | ((uint32_t)sizeBuffer[1] << 8) | ((uint32_t)sizeBuffer[2] << 16) | ((uint32_t)sizeBuffer[3] << 24);

    char* buffer = new char[length + 1];
    if (!ReadFile(pipe, (LPVOID)buffer, length, &bytesRead, NULL) || (bytesRead != length))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
        delete[] buffer;
        return false;
    }

    buffer[length] = '\0';
    s = buffer;

    delete[] buffer;
    return true;
}

void Client::ReadDataFromPipe()
{
    if (!IsValid())
    {
        HANDLE pipe = GetStdHandle(STD_INPUT_HANDLE);

        if (!ReadStringFromPipe(pipe, _data._name) ||
            !ReadStringFromPipe(pipe, _data._authToken) ||
            !ReadStringFromPipe(pipe, _data._refreshToken) ||
            !ReadStringFromPipe(pipe, _data._hostName) ||
            !ReadStringFromPipe(pipe, _data._leagueName) ||
            !ReadStringFromPipe(pipe, _data._leagueModName))
            return;

        CloseHandle(pipe);

        _data._participantID = 30;  //TODO: Get this value from the server API

        UpdateLeagueInfoText();
        UpdateVersionInfoText();
    }
}

void Client::UpdateVersionInfoText()
{
    typedef const char* (__thiscall* GetVersionProto)(void*);

    _versionInfoText.clear();

    GetVersionProto callback = (GetVersionProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    PULONG_PTR engine = EngineAPI::GetEngineHandle();

    if ((callback) && (engine))
    {
        const char* result = callback((void*)*engine);
        _versionInfoText = result;
        _versionInfoText += "\n{^F}GDCL v";
        _versionInfoText += GDCL_VERSION_MAJOR;
        _versionInfoText += ".";
        _versionInfoText += GDCL_VERSION_MINOR;
        _versionInfoText += ".";
        _versionInfoText += GDCL_VERSION_PATCH;
        _versionInfoText += " (";
        _versionInfoText += GetName();
        _versionInfoText += ")";
    }
}

void Client::UpdateLeagueInfoText()
{
    _leagueInfoText.clear();

    _leagueInfoText = L"\n";
    _leagueInfoText += std::wstring(_data._leagueName.begin(), _data._leagueName.end());
    _leagueInfoText += L"\n";
    _leagueInfoText += std::wstring(_data._name.begin(), _data._name.end());
    if ((_points > 0) && (_rank > 0))
    {
        _leagueInfoText += L" {^L}(Rank ";
        _leagueInfoText += std::to_wstring(_rank);
        _leagueInfoText += L" ~ ";
    }
    else
    {
        _leagueInfoText += L" {^L}(";
    }
    _leagueInfoText += std::to_wstring(_points);
    _leagueInfoText += L" points)";
}