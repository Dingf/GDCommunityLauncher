#include <atomic>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <future>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <Windows.h>
#include <CommCtrl.h>
#include "ServerAuth.h"
#include "UpdateDialog.h"
#include "ExeClient.h"
#include "HTTP.h"
#include "URI.h"
#include "Version.h"
#include "Log.h"

namespace UpdateDialog
{
    HWND _window = NULL;
    bool _result = false;
    std::shared_ptr<std::atomic_uint64_t> _totalSize = std::make_shared<std::atomic_uint64_t>(0);
    std::shared_ptr<std::atomic_uint64_t> _downloadSize = std::make_shared<std::atomic_uint64_t>(0);
}

typedef void (*DownloadValueCallback)(size_t);

bool GetDownloadList(std::unordered_map<std::wstring, std::string>& downloadList)
{
    HTTPRequest request(HTTP_METHOD_GET, "/File/filenames?branch=" + spClient->GetBranchName());
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());
 
    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                std::string seasonName = spClient->GetSeasonName();
                json responseJSON = json::parse(response.GetBody());

                for (const json& file : responseJSON)
                {
                    std::string filename = file.at("fileName").get<std::string>();
                    std::string downloadURL = file.at("downloadUrl").get<std::string>();
                    uint64_t fileSize = file.at("fileSize").get<uint64_t>();

                    // Generate the filename path based on the file extension and mod name
                    std::filesystem::path filenamePath(filename);
                    if (filenamePath.extension() == ".arc")
                        filenamePath = std::filesystem::current_path() / "mods" / seasonName / "resources" / filenamePath;
                    else if (filenamePath.extension() == ".arz")
                        filenamePath = std::filesystem::current_path() / "mods" / seasonName / "database" / filenamePath;
                    else
                        filenamePath = std::filesystem::current_path() / filenamePath;

                    // If the file doesn't exist or the file sizes don't match, add it to the list of files to download
                    if ((!std::filesystem::is_regular_file(filenamePath)) || (std::filesystem::file_size(filenamePath) != fileSize))
                        downloadList[filenamePath.wstring()] = downloadURL;
                }

                // Download the launcher as well if the version did not match previously
                const std::string& launcherURL = spClient->GetLauncherURL();
                if (!launcherURL.empty())
                {
                    std::filesystem::path filenamePath = std::filesystem::current_path() / "GDCommunityLauncher.zip";
                    downloadList[filenamePath.wstring()] = launcherURL;
                }
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve download file list: %", ex.what());
    }

    return false;
}

bool DownloadFile(const std::filesystem::path& filenamePath, const std::string& downloadURL, DownloadValueCallback totalSizeCallback, DownloadValueCallback downloadSizeCallback)
{
    try
    {
        std::string filename = filenamePath.filename().string();
        boost::replace_all(filename, " ", "%20");               // Replace spaces with '%20' for web URLs

        size_t index = downloadURL.find(".com/");
        if (index == std::string::npos)
        {
            throw std::runtime_error("Could not parse download URL");
            return false;
        }

        std::string host = downloadURL.substr(0, index + 4);
        std::string target = downloadURL.substr(index + 4);

        if (host.starts_with("https://"))    // Trim https:// if it's in the hostname
            host = host.substr(8);

        HTTPRequest request(HTTP_METHOD_GET, target);
        HTTPResponse response = request.Send(host, "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                size_t size = boost::lexical_cast<size_t>(response.GetHeaders().at("Content-Length"));
                totalSizeCallback(size);

                // Create the parent directory if it does not exist already
                std::filesystem::path parentPath = filenamePath.parent_path();
                if (!std::filesystem::is_directory(parentPath))
                    std::filesystem::create_directories(parentPath);

                std::filesystem::path tempPath = filenamePath;
                tempPath += ".tmp";

                std::ofstream out(tempPath, std::ofstream::binary | std::ofstream::out);
                auto in = response.GetStream();

                boost::system::error_code ec;
                do
                {
                    char buffer[1024];
                    size_t bytesRead = in->read_some(asio::buffer(buffer), ec);
                    if (!ec)
                    {
                        out.write(buffer, bytesRead);
                        downloadSizeCallback(bytesRead);
                    }
                }
                while (!ec);

                out.close();
                std::filesystem::rename(tempPath, filenamePath);
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download file %: %", filenamePath.filename(), ex.what());
    }

    return false;
}

bool VerifyBaseGameFiles(std::string& expectedVersion)
{
    std::vector<std::string> paths = { "database/database.arz", "gdx1/database/GDX1.arz", "gdx2/database/GDX2.arz" };

    json body = json::array();
    for (const auto& path : paths)
    {
        body.push_back({
            { "filename", path },
            { "filesize", std::filesystem::file_size(std::filesystem::current_path() / path) }
        });
    }

    HTTPRequest request(HTTP_METHOD_POST, "/File/base-game/file-sizes?branch=" + spClient->GetBranchName());
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());
    request.SetBody(body);

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
                return true;
            case 400:
            {
                expectedVersion = response.GetBody();
                throw std::runtime_error("File size mismatch. Server expects game version " + expectedVersion);
            }
            default:
                throw std::runtime_error("Server responed with status code " + response.GetStatus());
        }
    }
    catch (std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to verify base game files: %", ex.what());
    }

    return false;
}

void DownloadFiles(const std::unordered_map<std::wstring, std::string>& downloadList)
{
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
        uint64_t downloadSize = *UpdateDialog::_downloadSize;
        uint64_t totalSize = *UpdateDialog::_totalSize;

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
        std::string seasonName = spClient->GetSeasonName();
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

        std::unordered_map<std::wstring, std::string> downloadList;
        if (GetDownloadList(downloadList) && (downloadList.size() > 0))
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