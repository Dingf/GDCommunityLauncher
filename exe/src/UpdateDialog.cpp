#include <string>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <future>
#include <Windows.h>
#include <CommCtrl.h>
#include "Client.h"
#include "ServerAuth.h"
#include "UpdateDialog.h"
#include "dll/include/Exports/UpdateDialogExport.h"

namespace UpdateDialog
{
    HWND _window = NULL;
    bool _result = false;
    std::shared_ptr<size_t> _totalSize = std::make_shared<size_t>(0);
    std::shared_ptr<size_t> _downloadSize = std::make_shared<size_t>(0);
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
            std::string message = "The current Grim Dawn Community League requires Grim Dawn v" + std::string((const char*)lp) + " to play. Please switch your game version and try again.";
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