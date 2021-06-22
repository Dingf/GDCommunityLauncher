#include <windows.h>
#include "Client.h"

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    Client& client = Client::GetInstance();
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            client.SetupClientHooks();
            break;
        case DLL_PROCESS_DETACH:
            client.CLeanupClientHooks();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}