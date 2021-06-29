#include <windows.h>
#include "Client.h"
#include "Log.h"

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    Client& client = Client::GetInstance();
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (!client.SetupClientHooks())
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load GDCommunityLauncher.dll");
                return FALSE;
            }
            break;
        case DLL_PROCESS_DETACH:
            client.CleanupClientHooks();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}