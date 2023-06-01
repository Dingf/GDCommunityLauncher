#include <Windows.h>
#include <cpprest/http_client.h>
#include "Client.h"
#include "SelectorDialog.h"
#include "Date.h"
#include "Version.h"
#include "Log.h"

ClientData* data = nullptr;

bool HasBetaAccess(const std::string& role)
{
    return (role == "admin") || (role == "tester");
}

bool HasOffSeasonAccess(const std::string& role)
{
    return (HasBetaAccess(role)) || (role == "patreon_supporter");
}

std::string GetLauncherVersion(const ClientData& data)
{
    URI endpoint = data._gameURL / "File" / "launcher";
    endpoint.AddParam("branch", data._branch);

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + data._authToken;
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::value version = responseBody[U("version")];

                std::string versionString = JSONString(version.serialize());
                return versionString;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve launcher version: %", ex.what());
    }
    return {};
}

bool GetChatAPI(ClientData& data)
{
    URI endpoint = data._gameURL / "Admin" / "chat-url";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + data._authToken;
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                data._chatURL = response.extract_utf8string().get();
                return true;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve chat API: %", ex.what());
    }
    return false;
}

bool GetSeasonData(ClientData& data)
{
    URI endpoint = data._gameURL / "Season" / "latest";
    endpoint.AddParam("branch", data._branch);

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + data._authToken;
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::array seasonsList = responseBody.as_array();
                for (auto it = seasonsList.begin(); it != seasonsList.end(); ++it)
                {
                    std::string startDate = JSONString(it->at(U("startDate")).serialize());
                    std::string endDate = JSONString(it->at(U("endDate")).serialize());

                    std::time_t startDateTime = Date(startDate);
                    std::time_t endDateTime = Date(endDate);
                    std::time_t currentDateTime = Date();

                    if ((HasOffSeasonAccess(data._role)) || ((currentDateTime >= startDateTime) && (currentDateTime <= endDateTime)))
                    {
                        SeasonInfo seasonInfo;
                        seasonInfo._seasonID = it->at(U("seasonId")).as_integer();
                        seasonInfo._seasonType = it->at(U("seasonTypeId")).as_integer();

                        std::string modName = JSONString(it->at(U("modName")).serialize());
                        std::string displayName = JSONString(it->at(U("displayName")).serialize());
                        std::string participationToken = JSONString(it->at(U("participationTag")).serialize());

                        // Trim quotes from serializing the string
                        if ((modName.front() == '"') && (modName.back() == '"'))
                            modName = std::string(modName.begin() + 1, modName.end() - 1);
                        if ((displayName.front() == '"') && (displayName.back() == '"'))
                            displayName = std::string(displayName.begin() + 1, displayName.end() - 1);
                        if ((participationToken.front() == '"') && (participationToken.back() == '"'))
                            participationToken = std::string(participationToken.begin() + 1, participationToken.end() - 1);

                        for (char& c : participationToken)
                            c = std::tolower(c);

                        seasonInfo._modName = modName;
                        seasonInfo._displayName = displayName;
                        seasonInfo._participationToken = participationToken;

                        data._seasons.push_back(seasonInfo);
                    }
                }
                return true;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
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
                    if (data != nullptr)
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_RADIO1))
                            data->_branch = "prod";
                        else if (IsDlgButtonChecked(hwnd, IDC_RADIO2))
                            data->_branch = "beta";
                    }
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

bool SelectorDialog::Select(void* clientData)
{
    if (!clientData)
        return false;

    data = (ClientData*)clientData;
    if ((HasBetaAccess(data->_role)) && ((data->_branch.empty()) || (GetAsyncKeyState(VK_CONTROL) & (1 << 15))))
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

    if (!GetChatAPI(*data))
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve chat API information from the server.");
        return false;
    }

    if (!GetSeasonData(*data))
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve season information from the server.");
        return false;
    }
    
    data->_updateFlag = (GetLauncherVersion(*data) != std::string(GDCL_VERSION));
    
    return true;
}