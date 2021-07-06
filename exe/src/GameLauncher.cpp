#include <string>
#include <filesystem>
#include <windows.h>
#include "GameLauncher.h"
#include "Client.h"

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> TSTRING;

/*TSTRING operator+(const TCHAR* arg1, const std::filesystem::path& arg2)
{
#ifdef UNICODE
    return std::wstring(arg1) + arg2.wstring();
#else
    return std::string(arg1) + arg2.string();
#endif
}*/

bool InjectDLL(HANDLE process, const std::filesystem::path& dllPath)
{
    if (process)
    {
        LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryW");
        if (!loadLibraryAddress)
            return FALSE;

        std::wstring dllString = dllPath.wstring();
        LPVOID writeAddress = (LPVOID)VirtualAllocEx(process, NULL, dllString.length() * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!writeAddress)
            return FALSE;

        if (!WriteProcessMemory(process, writeAddress, dllString.c_str(), dllString.length() * sizeof(wchar_t), NULL))
            return FALSE;

        if (!CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, writeAddress, NULL, NULL))
            return FALSE;

        return TRUE;
    }
    return FALSE;
}

LPVOID BuildEnvironmentVariables()
{
    // Look for steam_api.dll to determine whether or not we are running on Steam
    std::filesystem::path installDir = std::filesystem::current_path();
    std::filesystem::path steamApiFile = installDir / "steam_api.dll";

    // If Steam is present, we need to build a whole slew of environment variables to bypass Steam's launcher
    if (std::filesystem::is_regular_file(steamApiFile))
    {
        std::filesystem::path commonDir = installDir.parent_path();
        std::filesystem::path steamappsDir = commonDir.parent_path();
        std::filesystem::path steamDir = steamappsDir.parent_path();

        std::vector<TSTRING> env =
        {
            TEXT("SteamEnv=1"),
            TEXT("SteamClientLaunch=1"),
            TEXT("SteamGameId=219990"),
            TEXT("SteamAppId=219990"),
            TEXT("SteamOverlayGameId=219990"),
            TEXT("SteamAppUser=username"),
            TEXT("SteamUser=username"),
            TEXT("STEAMID=00000000000000000"),
            TEXT("SESSIONNAME=Console"),
            /*TEXT("EnableConfiguratorSupport=0"),
            TEXT("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD=1"),
            TEXT("SDL_JOYSTICK_HIDAPI_STEAMXBOX=0"),
            TEXT("SteamStreamingHardwareEncodingNVIDIA=1"),
            TEXT("SteamStreamingHardwareEncodingAMD=1"),
            TEXT("SteamStreamingHardwareEncodingIntel=1"),
            TEXT("MESA_GLSL_CACHE_MAX_SIZE=5G"),
            TEXT("__GL_SHADER_DISK_CACHE_APP_NAME=steamapp_shader_cache"),
            TEXT("__GL_SHADER_DISK_CACHE_READ_ONLY_APP_NAME=steam_shader_cache;steamapp_merged_shader_cache"),
            TEXT("__GL_SHADER_DISK_CACHE_SKIP_CLEANUP=1"),
            TEXT("AMD_VK_PIPELINE_CACHE_FILENAME=steamapp_shader_cache"),
            TEXT("AMD_VK_USE_PIPELINE_CACHE=1"),
            TEXT("ENABLE_VK_LAYER_VALVE_steam_overlay_1=1"),
            TEXT("ENABLE_VK_LAYER_VALVE_steam_fossilize_1=1"),
            TEXT("MESA_DISK_CACHE_SINGLE_FILE=1"),
            TEXT("MESA_DISK_CACHE_READ_ONLY_FOZ_DBS=steam_cache"),*/
        };

        /*env.push_back(TEXT("INSTALLDIR=") + installDir);
        env.push_back(TEXT("ValvePlatformMutex=") + (steamDir / "steam.exe"));
        env.push_back(TEXT("MESA_GLSL_CACHE_DIR=") + (steamappsDir / "shadercache" / "219990"));
        env.push_back(TEXT("__GL_SHADER_DISK_CACHE_PATH=") + (steamappsDir / "shadercache" / "219990" / "nvidiav1"));
        env.push_back(TEXT("AMD_VK_PIPELINE_CACHE_PATH=") + (steamappsDir / "shadercache" / "219990" / "AMDv1"));
        env.push_back(TEXT("STEAM_FOSSILIZE_DUMP_PATH=") + (steamappsDir / "shadercache" / "219990" / "fozpipelinesv5" / "steamapprun_pipeline_cache"));
        env.push_back(TEXT("STEAM_FOSSILIZE_DUMP_PATH_READ_ONLY=") + (steamappsDir / "shadercache" / "219990" / "fozpipelinesv5" / "steam_pipeline_cache.foz"));
        env.push_back(TEXT("FOSSILIZE_APPLICATION_INFO_FILTER_PATH=") + (steamDir / "fossilize_engine_filters.json"));
        env.push_back(TEXT("STEAM_COMPAT_MEDIA_PATH=") + (steamappsDir / "shadercache" / "219990" / "fozmediav1"));
        env.push_back(TEXT("STEAM_COMPAT_TRANSCODED_MEDIA_PATH=") + (steamappsDir / "shadercache" / "219990" / "swarm"));
        env.push_back(TEXT("DXVK_STATE_CACHE_PATH=") + (steamappsDir / "shadercache" / "219990" / "DXVK_state_cache"));*/

        // Also make sure to preserve the current environment variables
        uint32_t start = 0;
        uint32_t index = 0;
        LPCH currentEnv = GetEnvironmentStrings();
        while (currentEnv[start] != '\0')
        {
            if (currentEnv[index] == '\0')
            {
                env.emplace_back(&currentEnv[start], index - start);
                start = index + 1;
            }
            index++;
        }
        env.push_back(GetEnvironmentStrings());

        // Calculate the total size and construct the environment buffer
        size_t totalSize = 1;
        for (uint32_t i = 0; i < env.size(); ++i)
        {
            totalSize += (env[i].length() + 1);
        }

        TCHAR* buffer = new TCHAR[totalSize];
        TCHAR* current = buffer;
        for (uint32_t i = 0; i < env.size(); ++i)
        {
            size_t length = env[i].length();
            memcpy(current, env[i].c_str(), sizeof(TCHAR) * length);
            current += length;
            *current++ = '\0';
        }
        buffer[totalSize - 1] = '\0';

        return buffer;
    }
    else
    {
        return NULL;
    }
    
}

HANDLE GameLauncher::LaunchProcess(const std::filesystem::path& exePath, const std::filesystem::path& dllPath)
{
    HANDLE pipeRead, pipeWrite;

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0) || !SetHandleInformation(pipeWrite, HANDLE_FLAG_INHERIT, FALSE))
        return NULL;

    STARTUPINFOW startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdInput = pipeRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    DWORD creationFlags = NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED;
#ifdef UNICODE
    creationFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

    LPVOID environment = BuildEnvironmentVariables();

    if (!CreateProcessW(exePath.c_str(), NULL, NULL, NULL, TRUE, creationFlags, environment, NULL, &startupInfo, &processInfo))
        return NULL;

    if (environment)
        delete[] environment;

    CloseHandle(pipeRead);

    Client& client = Client::GetInstance();
    if (!client.WriteDataToPipe(pipeWrite) || !InjectDLL(processInfo.hProcess, dllPath))
    {
        TerminateProcess(processInfo.hProcess, ERROR_ACCESS_DENIED);
        return NULL;
    }

    ResumeThread(processInfo.hThread);
    return processInfo.hProcess;
}