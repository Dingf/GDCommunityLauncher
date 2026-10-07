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
#include "Bitmap.h"
#include "LauncherCommon.h"
#include "CustomLayeredWindow.h"
#include "CustomButton.h"
#include "CustomLabel.h"
#include "CustomMessageBox.h"
#include "CustomProgressBar.h"
#include "DownloadWindow.h"
#include "ExeClient.h"
#include "FontManager.h"
#include "HTTP.h"
#include "Log.h"

class DownloadWindow : public CustomLayeredWindow
{
    public:
        static DownloadWindow* GetInstance();

        void AddToTotalSize(size_t size) { *_totalSize += size; }
        void AddToDownloadSize(size_t size) { *_downloadSize += size; }

        static void SetUpdateProgress();

        void RegisterWindow(HINSTANCE instance);
        void BuildWindow(HINSTANCE instance);


    private:
        DownloadWindow();

        static INT_PTR CALLBACK DownloadWindowHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw(HWND hwnd);

        std::shared_ptr<std::atomic_uint64_t> _totalSize;
        std::shared_ptr<std::atomic_uint64_t> _downloadSize;
};

#define spDownloadWindow DownloadWindow::GetInstance()

DownloadWindow::DownloadWindow()
{
    _totalSize = std::make_shared<std::atomic_uint64_t>(0);
    _downloadSize = std::make_shared<std::atomic_uint64_t>(0);
}

DownloadWindow* DownloadWindow::GetInstance()
{
    static DownloadWindow instance;
    return &instance;
}

typedef void (*DownloadValueCallback)(size_t);

bool GetDownloadList(std::unordered_map<std::wstring, std::string>& downloadList)
{
    HTTPRequest request(HTTP_METHOD_GET, "/File/filenames?branch=" + spClient->GetBranchName() + "&region=" + spClient->GetRegionName());
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

bool CheckLauncherUpdates()
{
    HTTPRequest request(HTTP_METHOD_GET, "/File/launcher?branch=" + spClient->GetBranchName() + "&region=" + spClient->GetRegionName());
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

bool VerifyBaseGameFiles()
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
                throw std::runtime_error("One or more files do not match the server");
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
        tasks.push_back(std::async(&DownloadFile, it.first, it.second, [](size_t value) { spDownloadWindow->AddToTotalSize(value); }, [](size_t value) { spDownloadWindow->AddToDownloadSize(value); }));
    }

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        if (tasks[i].get() == false)
        {
            SendMessage(spDownloadWindow->GetWindow(), WM_UPDATE_FAIL, NULL, NULL);
            return;
        }
    }

    SendMessage(spDownloadWindow->GetWindow(), WM_UPDATE_OK, NULL, NULL);
}

void DownloadWindow::SetUpdateProgress()
{
    while (IsWindow(spDownloadWindow->GetWindow()))
    {
        uint64_t downloadSize = *spDownloadWindow->_downloadSize;
        uint64_t totalSize = *spDownloadWindow->_totalSize;

        if (CustomLabel* progressLabel = dynamic_cast<CustomLabel*>(spDownloadWindow->GetWidget("progressLabel")))
        {
            std::wstringstream messageStream;
            messageStream << std::fixed << std::setprecision(1);
            messageStream << "Downloading files . . .  ";

            if (totalSize != 0)
            {
                size_t percent = (downloadSize * 100) / totalSize;
                messageStream << percent << "%";
            }

            progressLabel->SetText(messageStream.str());
        }

        if (CustomProgressBar* progressBar = dynamic_cast<CustomProgressBar*>(spDownloadWindow->GetWidget("progressBar")))
            progressBar->SetProgress((totalSize != 0) ?  (float)downloadSize / (float)totalSize : 0.0f);

        SendMessage(spDownloadWindow->GetWindow(), WM_PAINT, NULL, NULL);
    }
}

INT_PTR CALLBACK DownloadWindow::DownloadWindowHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_NCHITTEST:
        {
            static constexpr RECT titleBounds = { 0, 29, 404, 42 };

            POINT point = { LOWORD(lp), HIWORD(lp) };

            ScreenToClient(hwnd, &point);

            if (PtInRect(&titleBounds, point))
                return HTCAPTION; 

            break;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lp) == HTCAPTION) return TRUE;
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return TRUE;
        }
        case WM_CLOSE:
        {
            ExitProcess(EXIT_SUCCESS);
            return TRUE;
        }
        case WM_PAINT:
        {
            spDownloadWindow->Redraw(hwnd);
            return TRUE;
        }
        case WM_UPDATE_OK:
        {
            DestroyWindow(hwnd);
            return TRUE;
        }
        case WM_UPDATE_FAIL:
        {
            // Otherwise, notify the user that some of the files could not be downloaded
            CustomMessageBox::CreateMessageBox(hwnd, L"One or more files could not be downloaded. Check the GDCommunityLauncher.log file for more information.", MB_ICONERROR);
            ExitProcess(EXIT_SUCCESS);
            return TRUE;
        }
    }
    
    bool redraw = false;
    for (const auto& pair : spDownloadWindow->_widgets)
    {
        if (pair.second->HandleMessage(hwnd, msg, wp, lp))
            redraw = true;
    }

    if (redraw)
        SendMessage(hwnd, WM_PAINT, NULL, NULL);
        
    return DefWindowProc(hwnd, msg, wp, lp);
}

void DownloadWindow::RegisterWindow(HINSTANCE instance)
{
    WNDCLASSEX wc = { 0 };
    wc.lpfnWndProc = DownloadWindowHandler;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
    wc.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDB_ICON));
    wc.hCursor = LoadCursor(instance, MAKEINTRESOURCE(IDB_CURSOR));
    wc.lpszClassName = "GDCL_DownloadWindow";
    wc.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&wc);
}

void DownloadWindow::BuildWindow(HINSTANCE instance)
{
    if (_window = CreateWindowEx(WS_EX_LAYERED, "GDCL_DownloadWindow", "Grim Dawn Community Launcher", WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, instance, NULL))
    {
        HBITMAP background = LoadPNGBitmap(instance, MAKEINTRESOURCE(IDB_DOWNLOAD_BACKGROUND));
        BITMAP bitmap;
        GetObject(background, sizeof(BITMAP), &bitmap);
        SIZE size = { bitmap.bmWidth, bitmap.bmHeight };

        POINT origin = { 0 };
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

        POINT position =
        {
            monitorInfo.rcWork.left + (monitorInfo.rcWork.right  - monitorInfo.rcWork.left - bitmap.bmWidth) / 2,
            monitorInfo.rcWork.top  + (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top  - bitmap.bmHeight) / 2
        };

        HDC screenDC = GetDC(NULL);
        _buffer = CreateCompatibleDC(screenDC);
        SelectObject(_buffer, background);

        CustomWidget* progressBar = new CustomProgressBar(instance, _buffer, RECT(23, 94, 403, 120), IDB_PROGRESS_EMPTY, &CustomWidget::DefaultHandler);
        _widgets.emplace("progressBar", progressBar);

        CustomLabelSettings progressLabelSettings = { L"Arial", spFontManager->GetFont("Vinque"), Gdiplus::FontStyleBold, 15, 0xC0FFF0C0, 0xFF000000, L" ", nullptr };
        CustomWidget* progressLabel = new CustomLabel(instance, _buffer, RECT(112, 56, 392, 78), NULL, progressLabelSettings, &CustomWidget::DefaultHandler);
        _widgets.emplace("progressLabel", progressLabel);

        CustomButton* exitButton = new CustomButton(instance, _buffer, RECT(405, 33, 421, 49), IDB_DOWNLOAD_EXIT_UP, &CustomWidget::DefaultHandler);
        exitButton->SetClickHandler([](CustomWidget* _this, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> bool { SendMessage(hwnd, WM_CLOSE, NULL, NULL); return true; });
        _widgets.emplace("exit", exitButton);

        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        UpdateLayeredWindow(_window, screenDC, &position, &size, _buffer, &origin, RGB(0, 0, 0), &blend, ULW_ALPHA);

        ReleaseDC(NULL, screenDC);
    }
}

void DownloadWindow::Redraw(HWND hwnd)
{
    HDC screenDC = GetDC(NULL);
    POINT source = { 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    BITMAP bitmap;
    GetObject(GetCurrentObject(_buffer, OBJ_BITMAP), sizeof(BITMAP), &bitmap);
    SIZE size = { bitmap.bmWidth, bitmap.bmHeight };

    UpdateLayeredWindow(hwnd, screenDC, NULL, &size, _buffer, &source, RGB(0, 0, 0), &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

bool HandleDownloadWindow()
{
    if (!VerifyBaseGameFiles())
    {
        CustomMessageBox::CreateMessageBox(NULL, L"The Grim Dawn Community League requires the latest version of Grim Dawn with all expansions enabled to play. Please verify your game version and try again.", MB_ICONERROR);
        return false;
    }

    if (!CheckLauncherUpdates())
    {
        CustomMessageBox::CreateMessageBox(NULL, L"Could not retrieve launcher version from the server.", MB_ICONERROR);
        return false;
    }

    std::unordered_map<std::wstring, std::string> downloadList;
    if (!GetDownloadList(downloadList))
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not retrieve download list from the server.");
        return false;
    }

    if (downloadList.size() > 0)
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        spDownloadWindow->RegisterWindow(instance);
        spDownloadWindow->BuildWindow(instance);

        auto progressTask = std::async(DownloadWindow::SetUpdateProgress);
        auto downloadTask = std::async([&downloadList](){ DownloadFiles(downloadList); });

        MSG message;
        while (GetMessage(&message, 0, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    return true;
}