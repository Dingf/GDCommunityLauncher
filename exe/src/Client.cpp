#include "Client.h"

Client& Client::GetInstance(const std::string& name, const std::string& authToken)
{
    static Client instance(name, authToken);
    return instance;
}

bool Client::WriteDataToPipe(HANDLE pipe) const
{
    DWORD bytesWritten;
    char sizeBuffer[4];

    uint32_t nameLength = (uint32_t)_name.length();
    sizeBuffer[0] = nameLength & 0x000000FF;
    sizeBuffer[1] = (nameLength & 0x0000FF00) >> 8;
    sizeBuffer[2] = (nameLength & 0x00FF0000) >> 16;
    sizeBuffer[3] = (nameLength & 0xFF000000) >> 24;

    if (!WriteFile(pipe, sizeBuffer, 4, &bytesWritten, NULL) || (bytesWritten != 4))
        return FALSE;

    if (!WriteFile(pipe, _name.c_str(), nameLength, &bytesWritten, NULL) || (bytesWritten != nameLength))
        return FALSE;

    uint32_t authLength = (uint32_t)_authToken.length();
    sizeBuffer[0] = authLength & 0x000000FF;
    sizeBuffer[1] = (authLength & 0x0000FF00) >> 8;
    sizeBuffer[2] = (authLength & 0x00FF0000) >> 16;
    sizeBuffer[3] = (authLength & 0xFF000000) >> 24;

    if (!WriteFile(pipe, sizeBuffer, 4, &bytesWritten, NULL) || (bytesWritten != 4))
        return FALSE;

    if (!WriteFile(pipe, _authToken.c_str(), authLength, &bytesWritten, NULL) || (bytesWritten != authLength))
        return FALSE;

    CloseHandle(pipe);
    return TRUE;
}