#include <filesystem>
#include <boost/asio.hpp>
#include <Windows.h>
#include <gdiplus.h>
#include <tlhelp32.h>
#include "Configuration.h"
#include "FontManager.h"
#include "LoginWindow.h"
#include "DownloadWindow.h"
#include "GameLauncher.h"
#include "ContextManager.h"
#include "ExeClient.h"
#include "LauncherCommon.h"
#include "CustomMessageBox.h"
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

inline static bool RunLauncherTasks(Configuration* config, LPWSTR cmdLine)
{
    // Check to make sure that both the DLL and the GD executables are present in their relative paths
    std::filesystem::path gamePath, libraryPath;
    if (!VerifyLauncherPath(config, gamePath, libraryPath))
    {
        CustomMessageBox::CreateMessageBox(NULL, L"Both GDCommunityLauncher.exe and GDCommunityLauncher.dll must be located in the base Grim Dawn install directory.", MB_ICONERROR);
        return false;
    }

    // Display the login window or automatically login the user if autologin is enabled
    if (!HandleLoginWindow(config))
        return false;

    // Get the list of files from the server and download any files that need to be updated
    if ((!spClient->IsOfflineMode()) && (!HandleDownloadWindow()))
        return false;

    // Attempt to launch the process and hook into it with the DLL
    if (!GameLauncher::LaunchProcess(gamePath, libraryPath, cmdLine))
    {
        CustomMessageBox::CreateMessageBox(NULL, L"Failed to start the Grim Dawn Community Launcher. Check your log file for more details.", MB_ICONERROR);
        return false;
    }

    return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
    Logger::SetMinimumLogLevel(LOG_LEVEL_DEBUG);

    // Create a mutex to prevent multiple instances of the launcher from running
    std::wstring mutexName = std::wstring(L"Global\\GDCL_v") + GDCL_VERSION_WIDE;
    HANDLE mutex = CreateMutexW(NULL, TRUE, mutexName.c_str());

    if ((GetLastError() == ERROR_ALREADY_EXISTS) || (mutex == NULL))
    {
        if (mutex)
            CloseHandle(mutex);

        return EXIT_FAILURE;
    }

    // Load the launcher configuration from the .ini file
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    std::unique_ptr<Configuration> config = LoadConfiguration(configPath);

    ULONG_PTR gdipToken;
    Gdiplus::GdiplusStartupInput gdipStartup;
    Gdiplus::GdiplusStartup(&gdipToken, &gdipStartup, NULL);

    // Load custom fonts from resource
    HINSTANCE instance = GetModuleHandle(NULL);
    HRSRC fontVinque = FindResource(instance, MAKEINTRESOURCE(IDB_FONT_VINQUE), RT_FONT);
    spFontManager->LoadFontFromResource("Vinque", fontVinque);

    ContextManager::Run();

    bool result = RunLauncherTasks(config.get(), pCmdLine);

    config->Save(configPath);

    ContextManager::Stop();
    Gdiplus::GdiplusShutdown(gdipToken);

    ReleaseMutex(mutex);
    CloseHandle(mutex);

    return (result) ? EXIT_SUCCESS : EXIT_FAILURE;
}