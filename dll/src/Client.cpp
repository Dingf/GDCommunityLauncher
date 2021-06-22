#include <Windows.h>
#include "HookManager.h"
#include "Client.h"
#include "Log.h"

Client& Client::GetInstance()
{
    static Client instance;
    if (!instance._isInitialized)
    {
        instance.ReadClientDataFromPipe();
    }
    return instance;
}

void Client::ReadClientDataFromPipe()
{
    if (!_isInitialized)
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
        
        _isInitialized = true;
    }
}