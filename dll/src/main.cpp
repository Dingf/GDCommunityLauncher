#include <filesystem>
#include <Windows.h>
#include "DllClient.h"
#include "ChatHandler.h"
#include "GameHandler.h"
#include "ServerHandler.h"
#include "ServerCoordinator.h"
#include "ChatAPI.h"
#include "LuaAPI.h"
#include "Log.h"

bool InitializeModules()
{
    try { DllClient::GetInstance(); }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize DllClient module: %", ex.what());
        return false;
    }

    try {  GameHandler::GetInstance();  }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize GameHandler module: %", ex.what());
        return false;
    }

    try { ServerHandler::GetInstance(); }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ServerHandler module: %", ex.what());
        return false;
    }

    try { ChatHandler::GetInstance(); }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ChatHandler module: %", ex.what());
        return false;
    }

    try { ServerCoordinator::GetInstance(); }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ServerCoordinator module: %", ex.what());
        return false;
    }

    ChatAPI::Initialize();
    LuaAPI::Initialize();
    return true;
}

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    TCHAR buffer[MAX_PATH] = { 0 };
    if (!GetModuleFileName(NULL, buffer, MAX_PATH))
        return FALSE;

    Logger::SetMinimumLogLevel(LOG_LEVEL_DEBUG);

    std::filesystem::path processPath(buffer);
    if (processPath.filename() == "Grim Dawn.exe")
    {
        switch (fdwReason)
        {
            case DLL_PROCESS_ATTACH:
                if (!InitializeModules())
                    return FALSE;
                break;
            case DLL_PROCESS_DETACH:
                break;
            case DLL_THREAD_ATTACH:
                break;
            case DLL_THREAD_DETACH:
                break;
        }
    }
    return TRUE;
}