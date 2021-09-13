#include <string>
#include <sstream>
#include <filesystem>
#include <Windows.h>
#include <CommCtrl.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include "Configuration.h"
#include "UpdateDialog.h"
#include "JSONObject.h"
#include "URI.h"
#include "md5.hpp"

namespace UpdateDialog
{
    Configuration* _config = NULL;
    HWND _window = NULL;
    bool _result = false;
    std::shared_ptr<size_t> _totalSize = std::make_shared<size_t>(0);
    std::shared_ptr<size_t> _downloadSize = std::make_shared<size_t>(0);
}

std::string GenerateFileMD5(const std::filesystem::path& path)
{
    std::stringstream buffer;
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return {};

    buffer << in.rdbuf();

    // Capitalize the MD5 hash to match the server output
    std::string result = websocketpp::md5::md5_hash_hex(buffer.str());
    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        *it = toupper(*it);

    return result;
}

bool GetUpdateList(const std::string& hostName, std::vector<std::string>& updateList)
{
    URI endpoint = URI(hostName) / "api" / "File" / "filenames";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    //TODO: Replace me with the value from the API
    std::string modName = "GrimLeagueS03";

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
                    std::string checksum = JSONString(it->at(U("checksum")).serialize());

                    // Trim quotes from serializing the string
                    if ((filename.front() == '"') && (filename.back() == '"'))
                        filename = filename.substr(1, filename.size() - 2);
                    if ((checksum.front() == '"') && (checksum.back() == '"'))
                        checksum = checksum.substr(1, checksum.size() - 2);

                    // Capitalize the server MD5 hash for consistency
                    for (std::string::iterator it = checksum.begin(); it != checksum.end(); ++it)
                        *it = toupper(*it);

                    // Generate the filename path based on the file extension and mod name
                    std::filesystem::path filenamePath(filename);
                    if (filenamePath.extension() == ".arc")
                        filenamePath = std::filesystem::current_path() / "mods" / modName / "resources" / filenamePath;
                    else if (filenamePath.extension() == ".arz")
                        filenamePath = std::filesystem::current_path() / "mods" / modName / "database" / filenamePath;
                    else
                        filenamePath = std::filesystem::current_path() / filenamePath;

                    // If the file doesn't exist or the checksums don't match, add it to the list of files to download
                    if ((!std::filesystem::is_regular_file(filenamePath)) || (GenerateFileMD5(filenamePath) != checksum))
                        updateList.push_back(filename);
                }

                return true;
            }
            default:
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve file list: Server responded with status code %", response.status_code());
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve file list: %", ex.what());
    }

    return false;
}

void DownloadFiles(const std::string& hostName, const std::vector<std::string>& updateList)
{
    std::shared_ptr<size_t> totalSize = UpdateDialog::_totalSize;
    std::shared_ptr<size_t> downloadSize = UpdateDialog::_downloadSize;

    //TODO: Replace me with the value from the API
    std::string modName = "GrimLeagueS03";

    std::vector<pplx::task<bool>> tasks;
    for (size_t i = 0; i < updateList.size(); ++i)
    {
        const std::string& filename = updateList[i];
        tasks.push_back(pplx::create_task([totalSize, downloadSize, hostName, modName, filename]
        {
            URI endpoint = URI(hostName) / "api" / "File" / filename;
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            try
            {
                web::http::http_response response = httpClient.request(request).get();
                web::http::status_code status = response.status_code();

                if (status == web::http::status_codes::OK)
                {
                    *totalSize += response.headers().content_length();

                    // Generate the filename path based on the file extension and mod name
                    std::filesystem::path filenamePath(filename);
                    if (filenamePath.extension() == ".arc")
                        filenamePath = std::filesystem::current_path() / "mods" / modName / "resources" / filenamePath;
                    else if (filenamePath.extension() == ".arz")
                        filenamePath = std::filesystem::current_path() / "mods" / modName / "database" / filenamePath;
                    else
                        filenamePath = std::filesystem::current_path() / filenamePath;

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
                        *downloadSize += bytesRead;
                    }
                    while (bytesRead > 0);

                    fileStream.close().wait();

                    if (std::filesystem::is_regular_file(filenamePath))
                        std::filesystem::remove(filenamePath);

                    std::filesystem::rename(tempPath, filenamePath);

                    return true;
                }
                else
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download file %: Server responded with status code %", filename, response.status_code());
                    return false;
                }
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to download file %: %", filename, ex.what());
                return false;
            }
        }));
    }

    auto joinTask = pplx::when_all(std::begin(tasks), std::end(tasks))
        .then([](std::vector<bool> results)
        {
            if (std::all_of(results.begin(), results.end(), [](bool b) { return b; }))
            {
                SendMessage(UpdateDialog::_window, WM_UPDATE_OK, NULL, NULL);
            }
            else
            {
                SendMessage(UpdateDialog::_window, WM_UPDATE_FAIL, NULL, NULL);
            }
        });
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

bool UpdateDialog::Update(void* configPointer)
{
    if (!configPointer)
        return false;

    _config = (Configuration*)configPointer;

    std::string hostName;
    std::vector<std::string> updateList;

    const Value* hostValue = _config->GetValue("Login", "hostname");
    if ((hostValue) && (hostValue->GetType() == VALUE_TYPE_STRING))
        hostName = hostValue->ToString();

    if (!GetUpdateList(hostName, updateList))
        return false;

    if (updateList.size() > 0)
    {
        DownloadFiles(hostName, updateList);

        HINSTANCE instance = GetModuleHandle(NULL);
        _window = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG2), 0, UpdateDialogHandler, (LPARAM)_config);

        pplx::create_task(SetUpdateDialogProgress);

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            if (!IsDialogMessage(_window, &message))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        return _result;
    }
    else
        return true;
}