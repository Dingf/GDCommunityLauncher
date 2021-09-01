#include "Client.h"

Client& Client::GetInstance(const ServerData& data)
{
    static Client instance(data);
    return instance;
}

inline bool WriteStringToPipe(HANDLE pipe, const std::string& s)
{
    DWORD bytesWritten;
    char sizeBuffer[4];

    uint32_t length = (uint32_t)s.length();
    sizeBuffer[0] = length & 0x000000FF;
    sizeBuffer[1] = (length & 0x0000FF00) >> 8;
    sizeBuffer[2] = (length & 0x00FF0000) >> 16;
    sizeBuffer[3] = (length & 0xFF000000) >> 24;

    if (!WriteFile(pipe, sizeBuffer, 4, &bytesWritten, NULL) || (bytesWritten != 4))
        return false;

    if (!WriteFile(pipe, s.c_str(), length, &bytesWritten, NULL) || (bytesWritten != length))
        return false;

    return true;
}

bool Client::WriteDataToPipe(HANDLE pipe) const
{
    if (!WriteStringToPipe(pipe, _data._name) ||
        !WriteStringToPipe(pipe, _data._authToken) ||
        !WriteStringToPipe(pipe, _data._refreshToken) ||
        !WriteStringToPipe(pipe, _data._hostName) ||
        !WriteStringToPipe(pipe, _data._leagueName) ||
        !WriteStringToPipe(pipe, _data._leagueModName))
        return false;

    CloseHandle(pipe);
    return TRUE;
}