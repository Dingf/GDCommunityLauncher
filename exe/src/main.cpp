#include <filesystem>
#include <string>
#include <windows.h>

BOOL WriteClientData(HANDLE pipe, const std::string& clientName, const std::string& clientAuthToken)
{
    DWORD bytesWritten;
    char sizeBuffer[4];

    uint32_t nameLength = (uint32_t)clientName.length();
    sizeBuffer[0] =  nameLength & 0x000000FF;
    sizeBuffer[1] = (nameLength & 0x0000FF00) >> 8;
    sizeBuffer[2] = (nameLength & 0x00FF0000) >> 16;
    sizeBuffer[3] = (nameLength & 0xFF000000) >> 24;

    if (!WriteFile(pipe, sizeBuffer, 4, &bytesWritten, NULL) || (bytesWritten != 4))
        return FALSE;

    if (!WriteFile(pipe, clientName.c_str(), nameLength, &bytesWritten, NULL) || (bytesWritten != nameLength))
        return FALSE;

    uint32_t authLength = (uint32_t)clientAuthToken.length();
    sizeBuffer[0] =  authLength & 0x000000FF;
    sizeBuffer[1] = (authLength & 0x0000FF00) >> 8;
    sizeBuffer[2] = (authLength & 0x00FF0000) >> 16;
    sizeBuffer[3] = (authLength & 0xFF000000) >> 24;

    if (!WriteFile(pipe, sizeBuffer, 4, &bytesWritten, NULL) || (bytesWritten != 4))
        return FALSE;

    if (!WriteFile(pipe, clientAuthToken.c_str(), authLength, &bytesWritten, NULL) || (bytesWritten != authLength))
        return FALSE;

    CloseHandle(pipe);
    return TRUE;
}

BOOL InjectDLL(HANDLE process, LPCSTR dllPath)
{
    LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddress)
        return FALSE;

    SIZE_T dllPathLength = lstrlen(dllPath);
    LPVOID writeAddress = (LPVOID)VirtualAllocEx(process, NULL, dllPathLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!writeAddress)
        return FALSE;

    if (!WriteProcessMemory(process, writeAddress, dllPath, dllPathLength, NULL))
        return FALSE;

    if (!CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, writeAddress, NULL, NULL))
        return FALSE;

    return TRUE;
}

BOOL LaunchProcess(const std::string& exePath, const std::string& dllPath, const std::string& clientName, const std::string& clientAuthToken)
{
    HANDLE pipeRead, pipeWrite;

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0) || !SetHandleInformation(pipeWrite, HANDLE_FLAG_INHERIT, FALSE))
        return FALSE;

    STARTUPINFO startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdInput = pipeRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(exePath.c_str(), NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
        return FALSE;

    CloseHandle(pipeRead);
    
    if (!WriteClientData(pipeWrite, clientName, clientAuthToken) ||
        !InjectDLL(processInfo.hProcess, dllPath.c_str()))
    {
        TerminateProcess(processInfo.hProcess, ERROR_ACCESS_DENIED);
        return FALSE;
    }

    ResumeThread(processInfo.hThread);
    return TRUE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
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
        MessageBox(NULL, TEXT("Failed to start launcher. Both GDCommunityLauncher.exe and GDCommunityLauncher.dll must be located in the base Grim Dawn install directory."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    if (!LaunchProcess(grimDawnPath.string().c_str(), libraryPath.string().c_str(), "TestUser", "TestAuthToken"))
    {
        //TODO: Replace me with a more useful error message that gets the last error code from Windows
        MessageBox(NULL, TEXT("Failed to start launcher."), NULL, MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    return 0;
}