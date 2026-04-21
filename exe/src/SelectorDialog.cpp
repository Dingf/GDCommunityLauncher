#include <boost/asio.hpp>
#include <Windows.h>
#include "ExeClient.h"
#include "SelectorDialog.h"
#include "HTTP.h"
#include "Log.h"

inline bool HasBetaAccess(const std::string& role)
{
    return (role == "admin") || (role == "tester");
}

bool CheckLauncherUpdates()
{
    HTTPRequest request(HTTP_METHOD_GET, "/File/launcher?branch=" + spClient->GetBranchName());
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                json body = json::parse(response.GetBody());
                std::string version = body.at("version").get<std::string>();
                if (version != GDCL_VERSION)
                {
                    std::string downloadURL = body.at("downloadUrl").get<std::string>();
                    spClient->SetLauncherURL(downloadURL);
                }
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()) + " " + response.GetBody());
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve launcher version: %", ex.what());
    }
    return false;
}

bool GetChatAPI()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Admin/chat-url");
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                spClient->SetChatURL(response.GetBody());
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve chat API: %", ex.what());
    }
    return false;
}

bool GetSeasonName()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Season/latest/season-name?branch=" + spClient->GetBranchName());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                spClient->SetSeasonName(response.GetBody());
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season name: %", ex.what());
    }
    return false;
}

bool GetSeasonData()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Season/latest?branch=" + spClient->GetBranchName());
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                json responseJSON = json::parse(response.GetBody());
                for (const json& season : responseJSON)
                {
                    SeasonInfo seasonInfo;
                    seasonInfo._seasonID = season.at("seasonId").get<uint32_t>();
                    seasonInfo._seasonType = season.at("seasonTypeId").get<SeasonType>();
                    seasonInfo._modName = season.at("modName").get<std::string>();
                    seasonInfo._displayName = season.at("displayName").get<std::string>();
                    seasonInfo._participationToken = season.at("participationTag").get<std::string>();

                    // Set the participation token to lower case for standardization
                    for (char& c : seasonInfo._participationToken)
                        c = std::tolower(c);

                    spClient->AddSeason(seasonInfo);
                }
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season data: %", ex.what());
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
                    if (IsDlgButtonChecked(hwnd, IDC_RADIO1))
                        spClient->SetBranch(SEASON_BRANCH_RELEASE);
                    else if (IsDlgButtonChecked(hwnd, IDC_RADIO2))
                        spClient->SetBranch(SEASON_BRANCH_BETA);

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
    if (HasBetaAccess(spClient->GetRole()))
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
    else if (spClient->GetBranch() == SEASON_BRANCH_BETA)
    {
        spClient->SetBranch(SEASON_BRANCH_RELEASE);
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
    
    if (!CheckLauncherUpdates())
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve launcher version from the server.");
        return false;
    }

    return true;
}