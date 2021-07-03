#include <string>
#include <Windows.h>
#include "Client.h"
#include "Configuration.h"
#include "LoginDialog.h"
#include "ServerAuth.h"

namespace
{

Configuration* dialogConfig = NULL;
HWND dialogWindow = NULL;

}

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

void SetConfigurationData(HWND hwnd, Configuration* config)
{
    if (config)
    {
        bool isRememberMeChecked = (IsDlgButtonChecked(hwnd, IDC_CHECK1) == BST_CHECKED);

        config->SetValue("Login", "autologin", isRememberMeChecked);
        if (isRememberMeChecked)
        {
            std::string username = GetFieldText(hwnd, IDC_EDIT1);
            std::string password = GetFieldText(hwnd, IDC_EDIT2);
            config->SetValue("Login", "username", username.c_str());
            config->SetValue("Login", "password", password.c_str());
        }
        else
        {
            config->SetValue("Login", "username", "");
            config->SetValue("Login", "password", "");
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

                    const Value* hostValue = dialogConfig->GetValue("Login", "hostname");
                    if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
                    {
                        ServerAuth::ValidateCredentials(hostValue->ToString(), username, password, LoginValidateCallback);
                        SetDialogState(hwnd, FALSE);
                    }
                    else
                    {
                        MessageBoxA(hwnd, "Could not resolve the server hostname.", "Error", MB_OK | MB_ICONERROR);
                    }
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
            HWND usernameField = GetDlgItem(hwnd, IDC_EDIT1);
            HWND passwordField = GetDlgItem(hwnd, IDC_EDIT2);
            if (!image || !usernameField || !passwordField)
                return FALSE;

            HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICON1));
            if (!icon)
                return FALSE;

            HANDLE logo = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, NULL);
            if (!logo)
                return FALSE;

            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);

            SendMessage(image, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)logo);
            SendMessage(usernameField, EM_LIMITTEXT, 64, NULL);
            SendMessage(passwordField, EM_LIMITTEXT, 64, NULL);

            Configuration* config = (Configuration*)lp;

            const Value* autoLoginValue = config->GetValue("Login", "autologin");
            if ((autoLoginValue) && (autoLoginValue->GetType() == VALUE_TYPE_BOOL) && (autoLoginValue->ToBool()))
            {
                const Value* usernameValue = config->GetValue("Login", "username");
                const Value* passwordValue = config->GetValue("Login", "password");

                if ((usernameValue) && (usernameValue->GetType() == VALUE_TYPE_STRING))
                    SetDlgItemText(hwnd, IDC_EDIT1, usernameValue->ToString());

                if ((passwordValue) && (passwordValue->GetType() == VALUE_TYPE_STRING))
                    SetDlgItemText(hwnd, IDC_EDIT2, passwordValue->ToString());

                HWND autoLoginCheckbox = GetDlgItem(hwnd, IDC_CHECK1);
                SendMessage(autoLoginCheckbox, BM_SETCHECK, BST_CHECKED, NULL);
            }

            return TRUE;
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
            SetConfigurationData(hwnd, dialogConfig);
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

bool LoginDialog::CreateLoginDialog(void* configPointer)
{
    if (!dialogWindow)
    {
        dialogConfig = (Configuration*)configPointer;

        HINSTANCE instance = GetModuleHandle(NULL);
        dialogWindow = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialogHandler, (LPARAM)dialogConfig);

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