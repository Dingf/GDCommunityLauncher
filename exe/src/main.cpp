#include "Client.h"
#include "GameLauncher.h"

#include "Log.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Check to make sure that both the DLL and the GD executables are present in their relative paths
    std::filesystem::path current = std::filesystem::current_path();
#ifdef _WIN64
    std::filesystem::path grimDawnPath = current / "x64" / "Grim Dawn.exe";
#else
    std::filesystem::path grimDawnPath = current / "Grim Dawn.exe";
#endif
    std::filesystem::path libraryPath = current / "GDCommunityLauncher.dll";

    if (!std::filesystem::is_regular_file(grimDawnPath) || !std::filesystem::is_regular_file(libraryPath))
    {
        MessageBox(NULL, TEXT("Both GDCommunityLauncher.exe and GDCommunityLauncher.dll must be located in the base Grim Dawn install directory."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    if (!GameLauncher::CreateLoginDialog())
    {
        MessageBox(NULL, TEXT("Failed to start the launcher process."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
    
    Client& client = Client::GetInstance("TestUser123", "TestAuthToken");
    if (!GameLauncher::LaunchProcess(client, grimDawnPath, libraryPath))
    {
        //TODO: Replace me with a more useful error message that gets the last error code from Windows
        MessageBox(NULL, TEXT("Failed to launch Grim Dawn."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    return 0;
}