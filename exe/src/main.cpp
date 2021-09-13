#include <filesystem>
#include <windows.h>
#include "Configuration.h"
#include "LoginDialog.h"
#include "UpdateDialog.h"
#include "GameLauncher.h"
#include "ServerAuth.h"

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

    // Load the launcher configuration from the .ini file
    Configuration config;
    std::filesystem::path configPath = current / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
    }
    else
    {
        // If the file doesn't exist, create it using some default values
        config.SetValue("Login", "hostname", "https://www.grimleague.com/");
        config.SetValue("Login", "username", "");
        config.SetValue("Login", "password", "");
        config.SetValue("Login", "autologin", false);
        config.Save(configPath);
    }

    // Get the list of files from the server and download any files that need to be updated
    if (!UpdateDialog::Update(&config))
        return EXIT_FAILURE;

    // Display the login window or automatically login the user if autologin is enabled
    if (!LoginDialog::Login(&config))
        return EXIT_FAILURE;

    config.Save(configPath);
    
    if (!GameLauncher::LaunchProcess(grimDawnPath, libraryPath))
    {
        //TODO: Replace me with a more useful error message that gets the last error code from Windows
        MessageBox(NULL, TEXT("Failed to launch Grim Dawn."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    return 0;
}