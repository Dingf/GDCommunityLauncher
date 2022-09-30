#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <future>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "Client.h"
#include "ServerAuth.h"
#include "UpdateDialog.h"
#include "Date.h"
#include "JSONObject.h"
#include "URI.h"
#include "Version.h"
#include "Log.h"

namespace UpdateDialog
{
    HWND _window = NULL;
    bool _result = false;
    std::shared_ptr<size_t> _totalSize = std::make_shared<size_t>(0);
    std::shared_ptr<size_t> _downloadSize = std::make_shared<size_t>(0);
}

typedef void(*DownloadValueCallback)(size_t);

std::string GetSeasonModName(const std::string& hostName, const std::string& authToken)
{
    URI endpoint = URI(hostName) / "api" / "Season" / "latest";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + authToken;
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

                // No longer needed, since role/time access is granted by the server
                //if ((currentDateTime >= startDateTime) && (currentDateTime <= endDateTime))
                {
                    // Mod name should be the same across all seasons, so return the first result that is currently active
                    std::string modName = JSONString(it->at(U("modName")).serialize());
                    return modName;
                }
            }
            return {};
        }
        default:
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season mod name: %", ex.what());
    }
    return {};
}

bool GetLauncherUpdate(const std::string& hostName, const std::string& authToken, std::unordered_map<std::wstring, std::string>& downloadList)
{
    std::string version = GetLauncherVersion(hostName);
    if (version != std::string(GDCL_VERSION))
    {
        URI endpoint = URI(hostName) / "api" / "File" / "launcher";
        endpoint.Append(std::string("?v="), false);
        endpoint += GDCL_VERSION;

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + authToken;
        request.headers().add(U("Authorization"), bearerToken.c_str());

        try
        {
            web::http::http_response response = httpClient.request(request).get();
            switch (response.status_code())
            {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                std::string filename = JSONString(responseBody[U("fileName")].serialize());
                std::string downloadURL = JSONString(responseBody[U("downloadUrl")].serialize());
                std::filesystem::path filenamePath = std::filesystem::current_path() / filename;
                downloadList[filenamePath.wstring()] = downloadURL;
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
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download launcher: %", ex.what());
            return false;
        }
    }
    return true;
}

bool GetDownloadList(const std::string& hostName, const std::string& modName, const std::string& authToken, std::unordered_map<std::wstring, std::string>& downloadList)
{
    // Check for updates to the launcher, and add them to the download list if they exist
    GetLauncherUpdate(hostName, authToken, downloadList);

    URI endpoint = URI(hostName) / "api" / "File" / "filenames";
    endpoint.Append(std::string("?v="), false);
    endpoint += GDCL_VERSION;

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + authToken;
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
        case web::http::status_codes::OK:
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::array fileList = responseBody.as_array();

            for (auto it = fileList.begin(); it != fileList.end(); ++it)
            {
                std::string filename = JSONString(it->at(U("fileName")).serialize());
                std::string downloadURL = JSONString(it->at(U("downloadUrl")).serialize());
                uintmax_t fileSize = std::stoull(JSONString(it->at(U("fileSize")).serialize()));

                // Generate the filename path based on the file extension and mod name
                std::filesystem::path filenamePath(filename);
                if (filenamePath.extension() == ".arc")
                    filenamePath = std::filesystem::current_path() / "mods" / modName / "resources" / filenamePath;
                else if (filenamePath.extension() == ".arz")
                    filenamePath = std::filesystem::current_path() / "mods" / modName / "database" / filenamePath;
                else
                    filenamePath = std::filesystem::current_path() / filenamePath;

                // If the file doesn't exist or the file sizes don't match, add it to the list of files to download
                if ((!std::filesystem::is_regular_file(filenamePath)) || (std::filesystem::file_size(filenamePath) != fileSize))
                    downloadList[filenamePath.wstring()] = downloadURL;
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
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve file list: %", ex.what());
    }

    return false;
}

bool DownloadFile(const std::filesystem::path& filenamePath, const std::string& downloadURL, DownloadValueCallback totalSizeCallback, DownloadValueCallback downloadSizeCallback)
{
    URI endpoint = URI(downloadURL);

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        web::http::status_code status = response.status_code();

        if (status == web::http::status_codes::OK)
        {
            totalSizeCallback(response.headers().content_length());

            // Create the parent directory if it does not exist already
            std::filesystem::path parentPath = filenamePath.parent_path();
            if (!std::filesystem::is_directory(parentPath))
                std::filesystem::create_directories(parentPath);

            std::filesystem::path tempPath = filenamePath;
            tempPath += ".tmp";

            concurrency::streams::ostream fileStream = concurrency::streams::fstream::open_ostream(tempPath).get();
            concurrency::streams::istream body = response.body();

            size_t bytesRead = 0;
            do
            {
                bytesRead = body.read(fileStream.streambuf(), 1024).get();
                downloadSizeCallback(bytesRead);
            } while (bytesRead > 0);

            fileStream.close().wait();
            std::filesystem::rename(tempPath, filenamePath);
            return true;
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download file %: %", filenamePath.filename(), ex.what());
        return false;
    }
}

bool VerifyBaseGameFiles(const std::string& hostName, const std::string& authToken, std::string& expectedVersion)
{
    std::vector<pplx::task<bool>> tasks;
    // TODO: Make this more scalable, like store it as a list in a file or something
    std::vector<std::string> paths = { "database/database.arz", "gdx1/database/GDX1.arz", "gdx2/database/GDX2.arz" };
    try
    {
        URI endpoint = URI(hostName) / "api" / "File" / "base-game" / "file-sizes";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::POST);

        uint32_t index = 0;
        web::json::value requestBody = web::json::value::array();
        for (std::filesystem::path path : paths)
        {
            web::json::value fileData;
            fileData[U("filename")] = JSONString(path.string());
            fileData[U("filesize")] = std::filesystem::file_size(std::filesystem::current_path() / path);
            requestBody[index++] = fileData;
        }

        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + authToken;
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            return true;
        }
        else if (response.status_code() == web::http::status_codes::BadRequest)
        {
            expectedVersion = response.extract_utf8string().get();
            throw std::runtime_error("File size mismatch. Server expects game version " + expectedVersion);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to verify base game files: %", ex.what());
        return false;
    }
}

void DownloadFiles(const std::string& modName, const std::unordered_map<std::wstring, std::string>& updateList)
{
    std::shared_ptr<size_t> totalSize = UpdateDialog::_totalSize;
    std::shared_ptr<size_t> downloadSize = UpdateDialog::_downloadSize;

    std::vector<std::future<bool>> tasks;
    for (const auto& it : updateList)
    {
        const std::wstring& filename = it.first;
        const std::string& downloadURL = it.second;

        tasks.push_back(std::async(&DownloadFile, filename, downloadURL, [](size_t value) { *UpdateDialog::_totalSize += value; }, [](size_t value) { *UpdateDialog::_downloadSize += value; }));
    }

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        if (tasks[i].get() == false)
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_FAIL, NULL, NULL);
            return;
        }
    }

    SendMessage(UpdateDialog::_window, WM_UPDATE_OK, NULL, NULL);
}

INT_PTR CALLBACK UpdateDialogHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_COMMAND:
        {
            switch (wp)
            {
                case IDCANCEL:
                {
                    // Same as X/Alt-F4, don't start the game if Cancel is pressed
                    EndDialog(hwnd, 0);
                    ExitProcess(EXIT_SUCCESS);
                    return TRUE;
                }
            }
            return FALSE;
        }
        case WM_DESTROY:
        {
            // Set the window pointer to NULL to stop the update loop
            UpdateDialog::_window = NULL;
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
        case WM_UPDATE_OK:
        {
            // If all of the downloads succeeded, then gracefully exit and move on to the login window
            UpdateDialog::_result = true;
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_UPDATE_FAIL:
        {
            // Otherwise, notify the user that some of the files could not be downloaded
            MessageBoxA(hwnd, "One or more files could not be downloaded. Check the log for more information.", "Error", MB_OK | MB_ICONERROR);
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_UPDATE_WRONG_VERSION:
        {
            std::string message = "The current Grim Dawn Community League requires Ashes of Malmouth, Forgotten Gods, and Grim Dawn v" + std::string((const char*)lp) + " to play. Please verify your game version and try again.";
            MessageBoxA(hwnd, message.c_str(), "", MB_OK | MB_ICONINFORMATION);
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_UPDATE_NO_SEASON:
        {
            MessageBoxA(hwnd, "The Grim Dawn Community League is not currently active. Please visit https://www.grimleague.com for news about the upcoming season.", "", MB_OK | MB_ICONINFORMATION);
            DestroyWindow(hwnd);
            return TRUE;
        }
    }
    return FALSE;
}

void SetUpdateDialogProgress()
{
    while (UpdateDialog::_window)
    {
        size_t downloadSize = *UpdateDialog::_downloadSize;
        size_t totalSize = *UpdateDialog::_totalSize;

        HWND textField = GetDlgItem(UpdateDialog::_window, IDC_STATIC);
        HWND progressBar = GetDlgItem(UpdateDialog::_window, IDC_PROGRESS1);
        if (totalSize == 0)
        {
            SetWindowText(textField, "Checking for updates...");
            SendMessage(progressBar, PBM_SETPOS, 0, 0);
        }
        else
        {
            std::stringstream messageStream;
            messageStream.precision(2);

            size_t percent = (downloadSize * 100) / totalSize;
            messageStream << std::fixed << "Downloading files... " << percent << "% (";

            if (downloadSize >= 1e9)
                messageStream << (double)downloadSize / 1e9 << " GB";
            else if (downloadSize >= 1e6)
                messageStream << (double)downloadSize / 1e6 << " MB";
            else
                messageStream << (double)downloadSize / 1e3 << " KB";
                
            messageStream << " / ";

            if (totalSize >= 1e9)
                messageStream << (double)totalSize / 1e9 << " GB";
            else if (totalSize >= 1e6)
                messageStream << (double)totalSize / 1e6 << " MB";
            else
                messageStream << (double)totalSize / 1e3 << " KB";

            messageStream << ")";

            SetWindowText(textField, messageStream.str().c_str());
            SendMessage(progressBar, PBM_SETPOS, percent, 0);
        }

        Sleep(100);
    }
}

bool UpdateDialog::Update()
{
    HINSTANCE instance = GetModuleHandle(NULL);
    _window = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG2), 0, UpdateDialogHandler, NULL);

    auto progressTask = std::async(SetUpdateDialogProgress);
    auto updateTask   = std::async([]()
    {
        Client& client = Client::GetInstance();
        std::string hostName = client.GetHostName();
        std::string authToken = client.GetAuthToken();
        std::string modName = GetSeasonModName(hostName, authToken);
        if (modName.empty())
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_NO_SEASON, NULL, NULL);
            return;
        }

        std::string gameVersion;
        if (!VerifyBaseGameFiles(hostName, authToken, gameVersion))
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_WRONG_VERSION, NULL, (LPARAM)gameVersion.c_str());
            return;
        }

        std::unordered_map<std::wstring, std::string> downloadList;
        if (!GetDownloadList(hostName, modName, authToken, downloadList))
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_FAIL, NULL, NULL);
            return;
        }

        if (downloadList.size() > 0)
        {
            DownloadFiles(modName, downloadList);
        }
        else
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_OK, NULL, NULL);
        }
    });

    MSG message;
    while (GetMessage(&message, 0, 0, 0))
    {
        if (!IsDialogMessage(_window, &message))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    return UpdateDialog::_result;
}