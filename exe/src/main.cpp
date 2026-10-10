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

inline static std::unique_ptr<Configuration> LoadConfiguration(const std::filesystem::path& configPath)
{
    std::unique_ptr<Configuration> config = std::make_unique<Configuration>();
    if (std::filesystem::is_regular_file(configPath))
    {
        config->Load(configPath);
    }
    else
    {
        // If the file doesn't exist, create it using some default values
        config->SetValue("Login", "hostname", DEFAULT_HOST_NAME);
        config->SetValue("Login", "username", "");
        config->SetValue("Login", "password", "");
        config->SetValue("Login", "autologin", false);
        config->SetValue("Login", "branch", "1");
        config->SetValue("Login", "region", "0");
        config->SetValue("Login", "compatibility_mode", false);
        config->Save(configPath);
    }
    return config;
}

inline static bool VerifyLauncherPath(Configuration* config, std::filesystem::path& gamePath, std::filesystem::path& libraryPath)
{
    std::filesystem::path current = std::filesystem::current_path();

    const Value* compatValue = config->GetValue("Login", "compatibility_mode");
    if ((compatValue) && (compatValue->GetType() == VALUE_TYPE_BOOL) && (compatValue->ToBool()))
        gamePath = current / "compat" / "Grim Dawn.exe";
    else
        gamePath = current / "x64" / "Grim Dawn.exe";

    libraryPath = current / "GDCommunityLauncher.dll";

    return (std::filesystem::is_regular_file(gamePath) && std::filesystem::is_regular_file(libraryPath));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
    Logger::SetMinimumLogLevel(LOG_LEVEL_DEBUG);

    // Load the launcher configuration from the .ini file
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    std::unique_ptr<Configuration> config = LoadConfiguration(configPath);

    // Check to make sure that both the DLL and the GD executables are present in their relative paths
    std::filesystem::path gamePath, libraryPath;
    if (!VerifyLauncherPath(config.get(), gamePath, libraryPath))
    {
        MessageBox(NULL, TEXT("Both GDCommunityLauncher.exe and GDCommunityLauncher.dll must be located in the base Grim Dawn install directory."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
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
    if (!HandleLoginWindow(config.get()))
    {
        ContextManager::Stop();
        return EXIT_FAILURE;
    }

    config->Save(configPath);

    // Get the list of files from the server and download any files that need to be updated
    if ((!spClient->IsOfflineMode()) && (!HandleDownloadWindow()))
    {
        ContextManager::Stop();
        return EXIT_FAILURE;
    }

    ContextManager::Stop();

    Gdiplus::GdiplusShutdown(gdipToken);

    if (!GameLauncher::LaunchProcess(gamePath, libraryPath, pCmdLine))
    {
        MessageBox(NULL, TEXT("Failed to launch Grim Dawn."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}