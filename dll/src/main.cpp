#include <filesystem>
#include <windows.h>
#include "DllClient.h"
#include "GameHandler.h"
#include "ServerCoordinator.h"
#include "ServerHandler.h"
#include "ChatManager.h"
#include "Log.h"

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
                if (!DllClient::Initialize() ||
                    !GameHandler::Initialize() ||
                    !ServerHandler::Initialize() ||
                    !ServerCoordinator::Initialize() ||
                    !ChatManager::Initialize())
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