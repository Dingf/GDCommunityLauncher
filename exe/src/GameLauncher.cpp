#include <string>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <minizip/unzip.h>
#include "GameLauncher.h"
#include "Client.h"
#include "Log.h"

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> TSTRING;

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
        };

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

bool ExtractZIPUpdate()
{
    std::filesystem::path path = std::filesystem::current_path() / "GDCommunityLauncher.zip";
    std::string pathString = path.string();
    unzFile zipFile = unzOpen(pathString.c_str());
    if ((zipFile) && (unzLocateFile(zipFile, "GDCommunityLauncher.dll", 0) != UNZ_END_OF_LIST_OF_FILE))
    {
        std::filesystem::path filenamePath = std::filesystem::current_path() / "GDCommunityLauncher.dll";
        std::filesystem::path tempPath = filenamePath;
        tempPath += ".tmp";

        std::ofstream out(tempPath, std::ifstream::binary | std::ifstream::out);

        if ((!out.is_open()) || (unzOpenCurrentFile(zipFile) != UNZ_OK))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to extract files from \"%\"", pathString);
            return false;
        }

        int bytesRead = 0;
        char buffer[1024];
        do
        {
            bytesRead = unzReadCurrentFile(zipFile, buffer, 1024);
            if (bytesRead > 0)
            {
                out.write(buffer, bytesRead);
            }
        } while (bytesRead > 0);

        out.close();
        unzCloseCurrentFile(zipFile);
        unzClose(zipFile);

        std::filesystem::rename(tempPath, filenamePath);

        return true;
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open \"%\" for updating", pathString);
        return false;
    }
}

HANDLE GameLauncher::LaunchProcess(const std::filesystem::path& exePath, const std::filesystem::path& dllPath, LPWSTR cmdArgs)
{
    Client& client = Client::GetInstance();

    // Need to disconnect manually here because SignalR is dumb and will cause the process to hang if it's done in the destructor
    if (Connection* connection = client.GetConnection())
        connection->Disconnect();

    // If we need to update the launcher, unload the DLL and then overwrite it with the copy from the .zip file
    if (client.HasLauncherUpdate() && !ExtractZIPUpdate())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to update GDCommunityLauncher.dll");
        return NULL;
    }

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

    if (!CreateProcessW(exePath.c_str(), cmdArgs, NULL, NULL, TRUE, creationFlags, environment, NULL, &startupInfo, &processInfo))
        return NULL;

    if (environment)
        delete[] environment;

    CloseHandle(pipeRead);

    if (!client.WriteDataToPipe(pipeWrite) || !InjectDLL(processInfo.hProcess, dllPath))
    {
        TerminateProcess(processInfo.hProcess, ERROR_ACCESS_DENIED);
        return NULL;
    }

    ResumeThread(processInfo.hThread);
    return processInfo.hProcess;
}