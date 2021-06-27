#include <string>
#include <filesystem>
#include <windows.h>
#include "GameLauncher.h"
#include "Client.h"

bool InjectDLL(HANDLE process, const std::filesystem::path& dllPath)
{
    if (process)
    {
        LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
        if (!loadLibraryAddress)
            return FALSE;

        std::string dllString = dllPath.string();
        LPVOID writeAddress = (LPVOID)VirtualAllocEx(process, NULL, dllString.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!writeAddress)
            return FALSE;

        if (!WriteProcessMemory(process, writeAddress, dllString.c_str(), dllString.length(), NULL))
            return FALSE;

        if (!CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, writeAddress, NULL, NULL))
            return FALSE;

        return TRUE;
    }
    return FALSE;
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

    if (!CreateProcessW(exePath.c_str(), NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
        return NULL;

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