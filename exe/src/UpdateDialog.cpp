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

typedef void (*DownloadValueCallback)(size_t);

const std::unordered_map<std::wstring, std::string>& GetDownloadList()
{
    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        if (!connection->Invoke("GetLeagueFiles", client.GetAuthToken(), client.GetBranchName()))
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_FAIL, NULL, NULL);
        }
    }
    return client.GetDownloadList();
}

bool DownloadFile(const std::filesystem::path& filenamePath, const URI& downloadURL, DownloadValueCallback totalSizeCallback, DownloadValueCallback downloadSizeCallback)
{
    web::http::client::http_client httpClient((utility::string_t)downloadURL);
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
            }
            while (bytesRead > 0);

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

bool VerifyBaseGameFiles(std::string& expectedVersion)
{
    std::vector<pplx::task<bool>> tasks;
    // TODO: Make this more scalable, like store it as a list in a file or something
    std::vector<std::string> paths = { "database/database.arz", "gdx1/database/GDX1.arz", "gdx2/database/GDX2.arz" };
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "File" / "base-game" / "file-sizes";
        endpoint.AddParam("branch", client.GetBranchName());

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

        std::string bearerToken = "Bearer " + client.GetAuthToken();
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

void DownloadFiles(const std::unordered_map<std::wstring, std::string>& downloadList)
{
    std::shared_ptr<size_t> totalSize = UpdateDialog::_totalSize;
    std::shared_ptr<size_t> downloadSize = UpdateDialog::_downloadSize;

    std::vector<std::future<bool>> tasks;
    for (const auto& it : downloadList)
    {
        tasks.push_back(std::async(&DownloadFile, it.first, it.second, [](size_t value) { *UpdateDialog::_totalSize += value; }, [](size_t value) { *UpdateDialog::_downloadSize += value; }));
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
        std::string seasonName = client.GetSeasonName();
        if (seasonName.empty())
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_NO_SEASON, NULL, NULL);
            return;
        }

        std::string gameVersion;
        if (!VerifyBaseGameFiles(gameVersion))
        {
            SendMessage(UpdateDialog::_window, WM_UPDATE_WRONG_VERSION, NULL, (LPARAM)gameVersion.c_str());
            return;
        }

        const std::unordered_map<std::wstring, std::string>& downloadList = GetDownloadList();
        if (downloadList.size() > 0)
        {
            DownloadFiles(downloadList);
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