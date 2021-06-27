#include <string>
#include <Windows.h>
#include "Client.h"
#include "LoginDialog.h"
#include "ServerAuth.h"

HWND dialogWindow = NULL;

std::string GetFieldText(HWND hwnd, int id)
{
    HWND field = GetDlgItem(hwnd, id);
    uint32_t length = GetWindowTextLength(hwnd);
    std::string result(length, '\0');
    GetWindowText(field, &result[0], length);
    result.erase(result.find_last_not_of('\0') + 1);
    return result;
}

void SetDialogState(HWND hwnd, BOOL state)
{
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT1), state);
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT2), state);
    EnableWindow(GetDlgItem(hwnd, IDC_CHECK1), state);
    EnableWindow(GetDlgItem(hwnd, IDC_CHECK2), state);
    EnableWindow(GetDlgItem(hwnd, IDOK), state);
    EnableWindow(GetDlgItem(hwnd, IDHELP), state);
}

void LoginValidateCallback(ServerAuthResult result)
{
    if (dialogWindow)
    {
        switch (result)
        {
            case SERVER_AUTH_OK:
                SendMessage(dialogWindow, WM_LOGIN_OK, NULL, NULL);
                break;
            case SERVER_AUTH_FAIL:
                SendMessage(dialogWindow, WM_LOGIN_FAIL, NULL, NULL);
                break;
            case SERVER_AUTH_TIMEOUT:
                SendMessage(dialogWindow, WM_LOGIN_TIMEOUT, NULL, NULL);
                break;
        }
    }
}

INT_PTR CALLBACK LoginDialogHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_COMMAND:
        {
            switch (wp)
            {
                case IDOK:
                {
                    std::string username = GetFieldText(hwnd, IDC_EDIT1);
                    std::string password = GetFieldText(hwnd, IDC_EDIT2);
                    ServerAuth::ValidateCredentials(username, password, LoginValidateCallback);
                    SetDialogState(hwnd, FALSE);
                    return TRUE;
                }
                case IDHELP:
                {
                    return TRUE;
                }
            }
            return FALSE;
        }
        case WM_INITDIALOG:
        {
            HWND image = GetDlgItem(hwnd, IDC_IMAGE1);
            if (image)
            {
                SendMessage(image, STM_SETIMAGE, IMAGE_BITMAP, lp);
                return TRUE;
            }
            return FALSE;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return TRUE;
        }
        case WM_CLOSE:
        {
            // When the X button/Alt-F4 is pressed, exit gracefully and don't attempt to start the game
            EndDialog(hwnd, 0);
            ExitProcess(EXIT_SUCCESS);
            return TRUE;
        }
        case WM_LOGIN_OK:
        {
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_LOGIN_FAIL:
        {
            MessageBoxA(hwnd, "The username and/or password was incorrect.", "Error", MB_OK | MB_ICONERROR);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
        case WM_LOGIN_TIMEOUT:
        {
            MessageBoxA(hwnd, "Could not connect to the server.", "Error", MB_OK | MB_ICONERROR);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

bool LoginDialog::CreateLoginDialog()
{
    if (!dialogWindow)
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        HANDLE splashIcon = LoadImage(instance, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, NULL);
        if (!splashIcon)
            return FALSE;

        dialogWindow = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialogHandler, (LPARAM)splashIcon);

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            if (!IsDialogMessage(dialogWindow, &message))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }

        Client& client = Client::GetInstance();
        return client.IsValid();
    }
    return FALSE;
}