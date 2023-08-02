#include <string>
#include <future>
#include <Windows.h>
#include "Client.h"
#include "Configuration.h"
#include "ServerAuth.h"
#include "LoginDialog.h"
#include "SelectorDialog.h"
#include "Version.h"

enum LoginResult
{
    LOGIN_RESULT_OK = 0,
    LOGIN_RESULT_INVALID_LOGIN = 1,
    LOGIN_RESULT_TIMEOUT = 2,
    LOGIN_RESULT_INVALID_SEASONS = 3,
    LOGIN_RESULT_OTHER_ERROR = 4,
};

namespace LoginDialog
{
    Configuration* _config = NULL;
    HWND _window = NULL;
    std::string _aboutMessage;
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

bool InitializeClient(const ClientData& data)
{
    if ((data._branch != "offline") && (!SelectorDialog::Select((void*)&data)))
        return false;

    if ((LoginDialog::_config) && (!data._branch.empty()))
        LoginDialog::_config->SetValue("Login", "branch", data._branch.c_str());

    Client& client = Client::GetInstance(data);
    return true;
}

void LoginValidateCallback(ServerAuthResult result, const ClientData& data)
{
    if (LoginDialog::_window)
    {
        switch (result)
        {
            case SERVER_AUTH_OK:
            {
                if (!InitializeClient(data))
                    SendMessage(LoginDialog::_window, WM_LOGIN_OTHER_ERROR, NULL, NULL);
                else if (data._seasons.empty())
                    SendMessage(LoginDialog::_window, WM_LOGIN_INVALID_SEASONS, NULL, NULL);
                else
                    SendMessage(LoginDialog::_window, WM_LOGIN_OK, NULL, NULL);
                break;
            }
            case SERVER_AUTH_INVALID_LOGIN:
                SendMessage(LoginDialog::_window, WM_LOGIN_INVALID_LOGIN, NULL, NULL);
                break;
            case SERVER_AUTH_TIMEOUT:
                SendMessage(LoginDialog::_window, WM_LOGIN_TIMEOUT, NULL, NULL);
                break;
        }
    }
}

void SetConfigurationData(HWND hwnd, Configuration* config)
{
    if (config)
    {
        bool isRememberMeChecked = IsDlgButtonChecked(hwnd, IDC_CHECK1);

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

void DisplayLoginErrorMessageBox(HWND hwnd, LoginResult result)
{
    switch (result)
    {
        case LOGIN_RESULT_INVALID_LOGIN:
            MessageBoxA(hwnd, "The username and/or password was incorrect.", "Error", MB_OK | MB_ICONERROR);
            break;

        case LOGIN_RESULT_TIMEOUT:
            MessageBoxA(hwnd, "Could not connect to the server.", "Error", MB_OK | MB_ICONERROR);
            break;

        case LOGIN_RESULT_INVALID_SEASONS:
            MessageBoxA(hwnd, "The Grim Dawn Community League is not currently active. Please visit https://www.grimleague.com for news about the upcoming season.", "", MB_OK | MB_ICONINFORMATION);
            break;

        case LOGIN_RESULT_OTHER_ERROR:
            MessageBoxA(hwnd, "Could not initialize the Grim Dawn Community Launcher.", "Error", MB_OK | MB_ICONERROR);
            break;
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

                    const Value* hostValue = LoginDialog::_config->GetValue("Login", "hostname");
                    if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
                    {
                        ClientData data;
                        
                        data._username = username;
                        data._password = password;
                        data._gameURL = hostValue->ToString();

                        std::thread t(&ServerAuthenticate, data, LoginValidateCallback);
                        t.detach();

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
                    ClientData data;
                    data._branch = "offline";
                    data._updateFlag = false;

                    if (!InitializeClient(data))
                        return FALSE;

                    SendMessage(LoginDialog::_window, WM_LOGIN_OFFLINE_MODE, NULL, NULL);
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

            HINSTANCE launcherEXE = GetModuleHandle(NULL);
            HICON icon = LoadIcon(launcherEXE, MAKEINTRESOURCE(IDB_ICON1));
            if (!icon)
                return FALSE;

            HANDLE logo = LoadImage(launcherEXE, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, NULL);
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
            SetConfigurationData(hwnd, LoginDialog::_config);
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_LOGIN_OFFLINE_MODE:
        {
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_LOGIN_INVALID_LOGIN:
        {
            DisplayLoginErrorMessageBox(hwnd, LOGIN_RESULT_INVALID_LOGIN);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
        case WM_LOGIN_TIMEOUT:
        {
            DisplayLoginErrorMessageBox(hwnd, LOGIN_RESULT_TIMEOUT);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
        case WM_LOGIN_INVALID_SEASONS:
        {
            DisplayLoginErrorMessageBox(hwnd, LOGIN_RESULT_INVALID_SEASONS);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
        case WM_LOGIN_OTHER_ERROR:
        {
            DisplayLoginErrorMessageBox(hwnd, LOGIN_RESULT_OTHER_ERROR);
            SetDialogState(hwnd, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

bool LoginDialog::Login(void* configPointer)
{
    if (!configPointer)
        return false;

    _config = (Configuration*)configPointer;

    // Try to autologin if enabled, otherwise display the login prompt
    bool autoLogin = false;
    const Value* autoLoginValue = _config->GetValue("Login", "autologin");
    if ((autoLoginValue) && (autoLoginValue->GetType() == VALUE_TYPE_BOOL) && (autoLoginValue->ToBool()))
    {
        std::string hostName;
        const Value* hostValue = _config->GetValue("Login", "hostname");
        if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
            hostName = hostValue->ToString();

        std::string username;
        const  Value* usernameValue = _config->GetValue("Login", "username");
        if ((usernameValue) && (usernameValue->GetType() == VALUE_TYPE_STRING))
            username = usernameValue->ToString();

        std::string password;
        const Value* passwordValue = _config->GetValue("Login", "password");
        if ((passwordValue) && (passwordValue->GetType() == VALUE_TYPE_STRING))
            password = passwordValue->ToString();

        std::string branch;
        const Value* branchValue = _config->GetValue("Login", "branch");
        if ((branchValue) && (branchValue->GetType() == VALUE_TYPE_STRING))
            branch = branchValue->ToString();

        if (branch == "offline")
        {
            ClientData data;
            data._branch = "offline";
            data._updateFlag = false;

            if (!InitializeClient(data))
            {
                DisplayLoginErrorMessageBox(NULL, LOGIN_RESULT_OTHER_ERROR);
                return false;
            }

            autoLogin = true;
        }
        else if ((!hostName.empty()) && (!username.empty()) && (!password.empty()))
        {
            ClientData data;
            data._username = username;
            data._password = password;
            data._gameURL = hostName;
            data._branch = branch;

            std::future<ServerAuthResult> future = std::async(&ServerAuthenticate, std::ref(data), nullptr);
            ServerAuthResult loginResult = future.get();
            if (loginResult == SERVER_AUTH_OK)
            {
                if (!InitializeClient(data))
                {
                    DisplayLoginErrorMessageBox(NULL, LOGIN_RESULT_OTHER_ERROR);
                    return false;
                }
                else if (data._seasons.empty())
                {
                    DisplayLoginErrorMessageBox(NULL, LOGIN_RESULT_INVALID_SEASONS);
                    return false;
                }
                autoLogin = true;
            }
            else
            {
                DisplayLoginErrorMessageBox(NULL, (LoginResult)loginResult);
                if (loginResult != SERVER_AUTH_INVALID_LOGIN)
                    return false;
            }
        }
    }

    if (autoLogin)
    {
        return true;
    }
    else if (!_window)
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        LoginDialog::_window = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialogHandler, (LPARAM)_config);

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            if (!IsDialogMessage(_window, &message))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }

        Client& client = Client::GetInstance();
        if (!client.IsValid())
        {
            MessageBox(NULL, TEXT("Failed to retrieve data from the server."), NULL, MB_OK | MB_ICONERROR);
            return FALSE;
        }
        return TRUE;
    }
    else
    {
        MessageBox(NULL, TEXT("Failed to start the launcher process."), NULL, MB_OK | MB_ICONERROR);
        return FALSE;
    }
}