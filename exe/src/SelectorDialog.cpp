#include <Windows.h>
#include "Client.h"
#include "SelectorDialog.h"
#include "Log.h"

inline bool HasBetaAccess(const std::string& role)
{
    return (role == "admin") || (role == "tester");
}

inline bool HasOffSeasonAccess(const std::string& role)
{
    return (HasBetaAccess(role)) || (role == "patreon_supporter");
}

bool CheckLauncherVersion()
{
    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        return connection->Invoke("GetLauncherFile", client.GetAuthToken(), client.GetBranchName());
    }
    return false;
}

bool GetChatAPI()
{
    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        return connection->Invoke("GetChatUrl");
    }
    return false;
}

bool GetSeasonName()
{
    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        return connection->Invoke("GetLatestSeasonName", client.GetBranchName());
    }
    return false;
}

bool GetSeasonData()
{
    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        return connection->Invoke("GetLatestSeason", client.GetAuthToken(), false, client.GetBranchName());
    }
    return false;
}

INT_PTR CALLBACK SelectorDialogHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            SendDlgItemMessage(hwnd, IDC_RADIO1, BM_SETCHECK, 1, 0);
            break;
        }
        case WM_COMMAND:
        {
            switch (wp)
            {
                case IDC_RADIO1:
                {
                    SendDlgItemMessage(hwnd, IDC_RADIO1, BM_SETCHECK, 1, 0);
                    SendDlgItemMessage(hwnd, IDC_RADIO2, BM_SETCHECK, 0, 0);
                    return TRUE;
                }
                case IDC_RADIO2:
                {
                    SendDlgItemMessage(hwnd, IDC_RADIO1, BM_SETCHECK, 0, 0);
                    SendDlgItemMessage(hwnd, IDC_RADIO2, BM_SETCHECK, 1, 0);
                    return TRUE;
                }
                case IDOK:
                {
                    Client& client = Client::GetInstance();
                    if (IsDlgButtonChecked(hwnd, IDC_RADIO1))
                        client.SetBranch(SEASON_BRANCH_RELEASE);
                    else if (IsDlgButtonChecked(hwnd, IDC_RADIO2))
                        client.SetBranch(SEASON_BRANCH_BETA);

                    DestroyWindow(hwnd);
                    return TRUE;
                }
                case IDCANCEL:
                {
                    EndDialog(hwnd, 0);
                    ExitProcess(EXIT_SUCCESS);
                    return TRUE;
                }
                
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
            EndDialog(hwnd, 0);
            ExitProcess(EXIT_SUCCESS);
            return TRUE;
        }
    }
    return FALSE;
}

bool SelectorDialog::Select()
{
    Client& client = Client::GetInstance();
    if (HasBetaAccess(client.GetRole()))
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        HWND hwnd = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG3), 0, SelectorDialogHandler, 0);

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            if (!IsDialogMessage(hwnd, &message))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
    }
    else if (client.GetBranch() == SEASON_BRANCH_BETA)
    {
        client.SetBranch(SEASON_BRANCH_RELEASE);
    }

    if (!GetChatAPI())
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve chat API information from the server.");
        return false;
    }

    if (!GetSeasonName())
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve mod name from the server.");
        return false;
    }

    if (!GetSeasonData())
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve season information from the server.");
        return false;
    }
    
    if (!CheckLauncherVersion())
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve launcher version from the server.");
        return false;
    }

    return true;
}