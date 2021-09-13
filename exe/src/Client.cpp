#include "Client.h"
#include "Log.h"

Client& Client::GetInstance(const ServerData& data)
{
    static Client instance(data);
    return instance;
}

bool WriteIntToPipe(HANDLE pipe, uint32_t value)
{
    DWORD bytesWritten;
    uint8_t buffer[4];

    buffer[0] =  value & 0x000000FF;
    buffer[1] = (value & 0x0000FF00) >> 8;
    buffer[2] = (value & 0x00FF0000) >> 16;
    buffer[3] = (value & 0xFF000000) >> 24;

    return (WriteFile(pipe, buffer, 4, &bytesWritten, NULL) && (bytesWritten == 4));
}

bool WriteStringToPipe(HANDLE pipe, const std::string& str)
{
    DWORD bytesWritten;
    uint32_t length = (uint32_t)str.length();

    if (!WriteIntToPipe(pipe, length))
        return false;

    if (!WriteFile(pipe, str.c_str(), length, &bytesWritten, NULL) || (bytesWritten != length))
        return false;

    return true;
}

bool Client::WriteDataToPipe(HANDLE pipe) const
{
    if (!WriteIntToPipe(pipe, _data._participantID) ||
        !WriteStringToPipe(pipe, _data._username) ||
        !WriteStringToPipe(pipe, _data._authToken) ||
        !WriteStringToPipe(pipe, _data._refreshToken) ||
        !WriteStringToPipe(pipe, _data._hostName) ||
        !WriteStringToPipe(pipe, _data._seasonName) ||
        !WriteStringToPipe(pipe, _data._seasonModName))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write client data to the stdin pipe.");
        return false;
    }

    CloseHandle(pipe);
    return TRUE;
}