#include <filesystem>
#include <boost/asio.hpp>
#include <Windows.h>
#include <gdiplus.h>
#include "Configuration.h"
#include "FontManager.h"
#include "LoginWindow.h"
#include "DownloadWindow.h"
#include "GameLauncher.h"
#include "ContextManager.h"
#include "ExeClient.h"
#include "LauncherCommon.h"
#include "Log.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
    Logger::SetMinimumLogLevel(LOG_LEVEL_DEBUG);

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
        config.SetValue("Login", "hostname", DEFAULT_HOST_NAME);
        config.SetValue("Login", "username", "");
        config.SetValue("Login", "password", "");
        config.SetValue("Login", "autologin", false);
        config.SetValue("Login", "branch", "1");
        config.SetValue("Login", "region", "0");
        config.Save(configPath);
    }

    ULONG_PTR gdipToken;
    Gdiplus::GdiplusStartupInput gdipStartup;
    Gdiplus::GdiplusStartup(&gdipToken, &gdipStartup, NULL);

    // Load custom fonts from resource
    HINSTANCE instance = GetModuleHandle(NULL);
    HRSRC fontVinque = FindResource(instance, MAKEINTRESOURCE(IDB_FONT_VINQUE), RT_FONT);
    spFontManager->LoadFontFromResource("Vinque", fontVinque);

    ContextManager::Run();

    // Display the login window or automatically login the user if autologin is enabled
    if (!HandleLoginWindow(&config))
    {
        ContextManager::Stop();
        return EXIT_FAILURE;
    }

    config.Save(configPath);

    // Get the list of files from the server and download any files that need to be updated
    if ((!spClient->IsOfflineMode()) && (!HandleDownloadWindow()))
    {
        ContextManager::Stop();
        return EXIT_FAILURE;
    }

    ContextManager::Stop();

    Gdiplus::GdiplusShutdown(gdipToken);

    if (!GameLauncher::LaunchProcess(grimDawnPath, libraryPath, pCmdLine))
    {
        MessageBox(NULL, TEXT("Failed to launch Grim Dawn."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}