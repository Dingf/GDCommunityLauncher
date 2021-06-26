#include <filesystem>
#include <windows.h>
#include "GameLauncher.h"
#include "Client.h"
#include "LoginPrompt.h"

INT_PTR CALLBACK LoginDialogHandler(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hWnd, ico1), STM_SETIMAGE, IMAGE_BITMAP, lp);
        return TRUE;
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return TRUE;
    }
    return FALSE;
}

bool GameLauncher::CreateLoginDialog()
{
    HINSTANCE instance = GetModuleHandle(NULL);
    HANDLE splashIcon = LoadImage(instance, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, NULL);
    if (!splashIcon)
        return FALSE;

    MSG message;
    HWND dialog = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialogHandler, (LPARAM)splashIcon);
    while (GetMessage(&message, 0, 0, 0))
    {
        if (!IsDialogMessage(dialog, &message))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    return TRUE;
}

BOOL InjectDLL(HANDLE process, const std::filesystem::path& dllPath)
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

bool GameLauncher::LaunchProcess(const Client& client, const std::filesystem::path& exePath, const std::filesystem::path& dllPath)
{
    HANDLE pipeRead, pipeWrite;

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0) || !SetHandleInformation(pipeWrite, HANDLE_FLAG_INHERIT, FALSE))
        return FALSE;

    STARTUPINFOW startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdInput = pipeRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcessW(exePath.c_str(), NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
        return FALSE;

    CloseHandle(pipeRead);

    if (!client.WriteDataToPipe(pipeWrite) || !InjectDLL(processInfo.hProcess, dllPath))
    {
        TerminateProcess(processInfo.hProcess, ERROR_ACCESS_DENIED);
        return FALSE;
    }

    ResumeThread(processInfo.hThread);
    return TRUE;
}