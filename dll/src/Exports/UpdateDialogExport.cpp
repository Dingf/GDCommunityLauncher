#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include "Exports/ServerAuthExport.h"
#include "Exports/UpdateDialogExport.h"
#include "Date.h"
#include "JSONObject.h"
#include "URI.h"
#include "Version.h"
#include "Log.h"

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