#include <filesystem>
#include <windows.h>
#include "Client.h"
#include "ClientHandler.h"

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    TCHAR buffer[MAX_PATH] = { 0 };
    if (!GetModuleFileName(NULL, buffer, MAX_PATH))
        return FALSE;

    std::filesystem::path processPath(buffer);
    if (processPath.filename() == "Grim Dawn.exe")
    {
        Client& client = Client::GetInstance();
        switch (fdwReason)
        {
            case DLL_PROCESS_ATTACH:
                client.Initialize();
                ClientHandler::CreateHooks();
                break;
            case DLL_PROCESS_DETACH:
                ClientHandler::DeleteHooks();
                break;
            case DLL_THREAD_ATTACH:
                break;
            case DLL_THREAD_DETACH:
                break;
        }
    }
    return TRUE;
}