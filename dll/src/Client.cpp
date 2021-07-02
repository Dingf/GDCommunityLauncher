#include <Windows.h>
#include "HookManager.h"
#include "Client.h"
#include "EngineAPI.h"
#include "Log.h"

Client& Client::GetInstance()
{
    static Client instance;
    if (!instance.IsValid())
    {
        instance.ReadDataFromPipe();
    }
    return instance;
}

void Client::ReadDataFromPipe()
{
    if (!IsValid())
    {
        HANDLE pipeIn = GetStdHandle(STD_INPUT_HANDLE);

        DWORD bytesRead;
        uint8_t sizeBuffer[4];
        if (!ReadFile(pipeIn, &sizeBuffer, 4, &bytesRead, NULL) || (bytesRead != 4))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            return;
        }

        uint32_t nameLength = (uint32_t)sizeBuffer[0] | ((uint32_t)sizeBuffer[1] << 8) | ((uint32_t)sizeBuffer[2] << 16) | ((uint32_t)sizeBuffer[3] << 24);

        char* nameBuffer = new char[nameLength + 1];
        if (!ReadFile(pipeIn, (LPVOID)nameBuffer, nameLength, &bytesRead, NULL) || (bytesRead != nameLength))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            delete[] nameBuffer;
            return;
        }

        nameBuffer[nameLength] = '\0';
        _name = nameBuffer;

        delete[] nameBuffer;

        if (!ReadFile(pipeIn, &sizeBuffer, 4, &bytesRead, NULL) || (bytesRead != 4))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            return;
        }

        uint32_t authLength = (uint32_t)sizeBuffer[0] | ((uint32_t)sizeBuffer[1] << 8) | ((uint32_t)sizeBuffer[2] << 16) | ((uint32_t)sizeBuffer[3] << 24);

        char* authBuffer = new char[authLength + 1];
        if (!ReadFile(pipeIn, (LPVOID)authBuffer, authLength, &bytesRead, NULL) || (bytesRead != authLength))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            delete[] authBuffer;
            return;
        }

        authBuffer[authLength] = '\0';
        _authToken = authBuffer;

        delete[] authBuffer;

        CloseHandle(pipeIn);

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
        _versionInfoText.append(result);
        _versionInfoText.append("\n{^F}GrimLeague S3 (");
        _versionInfoText.append(GetName());
        _versionInfoText.append(")");
    }
}

void Client::UpdateLeagueInfoText()
{
    _leagueInfoText.clear();

    _leagueInfoText = L"\nGrimLeague Season 3\n";
    _leagueInfoText += std::wstring(_name.begin(), _name.end());
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