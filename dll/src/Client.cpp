#include <filesystem>
#include <Windows.h>
#include "HookManager.h"
#include "Client.h"
#include "GameAPI.h"
#include "EngineAPI.h"
#include "Log.h"
#include "Character.h"
#include "Quest.h"
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

void Client::SetActiveSeason(const std::string& modName, bool hardcore)
{
    _activeSeason = nullptr;
    for (size_t i = 0; i < _data._seasons.size(); ++i)
    {
        SeasonInfo& season = _data._seasons[i];
        if ((modName == season._modName) && ((1 + hardcore) == season._seasonType))
        {
            _activeSeason = &season;
            break;
        }
    }
    UpdateLeagueInfoText();
}

void Client::SetActiveCharacter(const std::wstring& name, bool hasToken)
{
    _activeCharacter._name = name;
    _activeCharacter._hasToken = hasToken;
}

bool ReadIntFromPipe(HANDLE pipe, uint32_t& value)
{
    DWORD bytesRead;
    uint8_t buffer[4];

    if (!ReadFile(pipe, &buffer, 4, &bytesRead, NULL) || (bytesRead != 4))
        return false;

    value = (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 24);

    return true;
}

bool ReadStringFromPipe(HANDLE pipe, std::string& str)
{
    DWORD bytesRead;
    uint32_t length;

    if (!ReadIntFromPipe(pipe, length))
        return false;

    char* buffer = new char[length + 1];
    if (!ReadFile(pipe, (LPVOID)buffer, length, &bytesRead, NULL) || (bytesRead != length))
    {
        delete[] buffer;
        return false;
    }

    buffer[length] = '\0';
    str = buffer;

    delete[] buffer;
    return true;
}

bool ReadSeasonsFromPipe(HANDLE pipe, std::vector<SeasonInfo>& seasons)
{
    uint32_t count;
    if (!ReadIntFromPipe(pipe, count))
        return false;

    for (uint32_t i = 0; i < count; ++i)
    {
        SeasonInfo season;

        if (!ReadIntFromPipe(pipe, season._seasonID) ||
            !ReadIntFromPipe(pipe, season._seasonType) ||
            !ReadStringFromPipe(pipe, season._modName) ||
            !ReadStringFromPipe(pipe, season._displayName) ||
            !ReadStringFromPipe(pipe, season._participationToken))
            return false;

        seasons.push_back(season);
    }
    return true;
}

void Client::ReadDataFromPipe()
{
    if (!IsValid())
    {
        HANDLE pipe = GetStdHandle(STD_INPUT_HANDLE);

        if (!ReadStringFromPipe(pipe, _data._username) ||
            !ReadStringFromPipe(pipe, _data._authToken) ||
            !ReadStringFromPipe(pipe, _data._refreshToken) ||
            !ReadStringFromPipe(pipe, _data._hostName) ||
            !ReadSeasonsFromPipe(pipe, _data._seasons))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            return;
        }

        CloseHandle(pipe);

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
        _versionInfoText += GetUsername();
        _versionInfoText += ")";
    }
}

void Client::UpdateLeagueInfoText()
{
    _leagueInfoText.clear();

    if (_activeSeason)
    {
        _leagueInfoText = L"\n";
        _leagueInfoText += std::wstring(_activeSeason->_displayName.begin(), _activeSeason->_displayName.end());
    }
    _leagueInfoText += L"\n";
    _leagueInfoText += std::wstring(_data._username.begin(), _data._username.end());
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