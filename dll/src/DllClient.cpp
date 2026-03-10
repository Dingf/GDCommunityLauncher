#include <filesystem>
#include <Windows.h>
#include <minizip/unzip.h>
#include "DllClient.h"
#include "GameHandler.h"
#include "HookManager.h"
#include "EventManager.h"
#include "ThreadManager.h"
#include "DeathRecap.h"
#include "ServerSync.h"
#include "URI.h"
#include "Log.h"
#include "Version.h"

DllClient::DllClient() : _activeSeason(nullptr)
{
    ReadDataFromPipe();
}

DllClient* DllClient::GetInstance()
{
    static DllClient instance;
    return &instance;
}

bool DllClient::Initialize()
{
    try
    {
        GetInstance();
        return true;
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize DllClient module: %", ex.what());
        return false;
    }
}

bool ReadByteFromPipe(HANDLE pipe, uint8_t& value)
{
    DWORD bytesRead;
    if (!ReadFile(pipe, &value, 1, &bytesRead, NULL) || (bytesRead != 1))
        return false;

    return true;
}

bool ReadInt16FromPipe(HANDLE pipe, uint16_t& value)
{
    DWORD bytesRead;
    uint8_t buffer[2];

    if (!ReadFile(pipe, &buffer, 2, &bytesRead, NULL) || (bytesRead != 2))
        return false;

    value = (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8);

    return true;
}

bool ReadInt32FromPipe(HANDLE pipe, uint32_t& value)
{
    DWORD bytesRead;
    uint8_t buffer[4];

    if (!ReadFile(pipe, &buffer, 4, &bytesRead, NULL) || (bytesRead != 4))
        return false;

    value = (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 24);

    return true;
}

bool ReadStringFromPipe(HANDLE pipe, std::string& str)
{
    DWORD bytesRead;
    uint32_t length;

    if (!ReadInt32FromPipe(pipe, length))
        return false;

    if (length > 0)
    {
        char* buffer = new char[length + 1];
        if (!ReadFile(pipe, (LPVOID)buffer, length, &bytesRead, NULL) || (bytesRead != length))
        {
            delete[] buffer;
            return false;
        }

        buffer[length] = '\0';
        str = buffer;

        delete[] buffer;
    }
    return true;
}

bool ReadWideStringFromPipe(HANDLE pipe, std::wstring& str)
{
    uint32_t length;

    if (!ReadInt32FromPipe(pipe, length))
        return false;

    if (length > 0)
    {
        wchar_t* buffer = new wchar_t[length + 1];
        for (size_t i = 0; i < length; ++i)
        {
            if (!ReadInt16FromPipe(pipe, (uint16_t&)buffer[i]))
            {
                delete[] buffer;
                return false;
            }
        }

        buffer[length] = '\0';
        str = buffer;

        delete[] buffer;
    }
    return true;
}

bool ReadSeasonsFromPipe(HANDLE pipe, std::vector<SeasonInfo>& seasons)
{
    uint32_t count;
    if (!ReadInt32FromPipe(pipe, count))
        return false;

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t seasonType;
        SeasonInfo season;

        if (!ReadInt32FromPipe(pipe, season._seasonID) ||
            !ReadInt32FromPipe(pipe, seasonType) ||
            !ReadStringFromPipe(pipe, season._displayName) ||
            !ReadStringFromPipe(pipe, season._participationToken))
            return false;

        season._seasonType = (SeasonType)seasonType;
        seasons.push_back(season);
    }
    return true;
}

bool ExtractZIPUpdate()
{
    const std::filesystem::path& path = std::filesystem::current_path() / "GDCommunityLauncher.zip";
    std::string pathString = path.string();
    unzFile zipFile = unzOpen(pathString.c_str());
    if ((zipFile) && (unzLocateFile(zipFile, "GDCommunityLauncher.exe", 0) != UNZ_END_OF_LIST_OF_FILE))
    {
        std::filesystem::path filenamePath = std::filesystem::current_path() / "GDCommunityLauncher.exe";
        std::filesystem::path tempPath = filenamePath;
        tempPath += ".tmp";

        std::ofstream out(tempPath, std::ifstream::binary | std::ifstream::out);

        if ((!out.is_open()) || (unzOpenCurrentFile(zipFile) != UNZ_OK))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to extract files from \"%\"", pathString);
            return false;
        }

        int bytesRead = 0;
        char buffer[1024];
        do
        {
            bytesRead = unzReadCurrentFile(zipFile, buffer, 1024);
            if (bytesRead > 0)
            {
                out.write(buffer, bytesRead);
            }
        }
        while (bytesRead > 0);

        out.close();
        unzCloseCurrentFile(zipFile);
        unzClose(zipFile);

        std::filesystem::rename(tempPath, filenamePath);

        return true;
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open \"%\" for updating", pathString);
        return false;
    }
}

void DllClient::ReadDataFromPipe()
{
    HANDLE pipe = GetStdHandle(STD_INPUT_HANDLE);

    uint8_t updateFlag;
    std::string host;
    std::string chatURL;
    uint32_t branch;

    if (!ReadStringFromPipe(pipe, _username) ||
        !ReadStringFromPipe(pipe, _password) ||
        !ReadStringFromPipe(pipe, _authToken) ||
        !ReadStringFromPipe(pipe, _refreshToken) ||
        !ReadStringFromPipe(pipe, _seasonName) ||
        !ReadStringFromPipe(pipe, host) ||
        !ReadStringFromPipe(pipe, chatURL) ||
        !ReadInt32FromPipe(pipe, branch) ||
        !ReadByteFromPipe(pipe, updateFlag) ||
        !ReadSeasonsFromPipe(pipe, _seasons))
    {
        throw std::runtime_error("Could not read client data from stdin pipe.");
    }

    CloseHandle(pipe);

    _host = URI(host);
    _chatURL = URI(chatURL);
    _branch = static_cast<SeasonBranch>(branch);
    if ((updateFlag != 0) && (!ExtractZIPUpdate()))
    {
        throw std::runtime_error("Could not update GDCommunityLauncher.exe");
    }

    if (_seasons.size() > 0)
    {
        std::string rootPrefix = _seasonName;
        if (_branch != SEASON_BRANCH_RELEASE)
        {
            rootPrefix += "_";
            rootPrefix += GetBranchName();
        }

        GameAPI::SetRootPrefix(rootPrefix);
    }
}

void DllClient::SetActiveSeason(bool hardcore)
{
    _activeSeason = nullptr;
    for (size_t i = 0; i < _seasons.size(); ++i)
    {
        SeasonInfo& season = _seasons[i];
        if ((1 + hardcore) == season._seasonType)
        {
            _activeSeason = &season;
            break;
        }
    }
    //UpdateLeagueInfoText();
}

//void SeasonClient::SetParticipantID(uint32_t participantID)
//{
    //_participantID = participantID;
    //UpdateSeasonStanding();
//}

/*void Client::UpdateSeasonStanding()
{
    URI endpoint = GetServerGameURL() / "Season" / "participant" / std::to_string(GetCurrentParticipantID()) / "standing";
    web::http::http_request request(web::http::methods::GET);

    ServerSync::ScheduleTask([endpoint, request]()
    {
        web::http::client::http_client httpClient((utility::string_t)endpoint);
        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
            {
                response.extract_json().then([](web::json::value responseBody)
                {
                    try
                    {
                        web::json::value pointTotal = responseBody[U("pointTotal")];
                        web::json::value rank = responseBody[U("rank")];

                        Client& client = Client::GetInstance();
                        client._points = pointTotal.as_integer();
                        client._rank = rank.as_integer();
                        client.UpdateLeagueInfoText();
                    }
                    catch (const std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update season standing: %", ex.what());
                    }
                });
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update season standing: Server responded with status code %", response.status_code());
            }
        });
    });
}*/

/*int64_t Client::UpdateRefreshToken()
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = client.GetServerGameURL() / "Account" / "login";

        web::json::value requestBody;
        requestBody[U("username")] = JSONString(client._username);
        requestBody[U("password")] = JSONString(client._password);

        web::http::http_request request(web::http::methods::POST);
        request.set_body(requestBody);

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::value authTokenValue = responseBody[U("access_token")];
            web::json::value refreshTokenValue = responseBody[U("refresh_token")];
            if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
            {
                Client& client = Client::GetInstance();
                client._authToken = JSONString(authTokenValue.serialize());
                client._refreshToken = JSONString(refreshTokenValue.serialize());
            }
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to refresh token: %", ex.what());
    }
    return 900000;
}*/

// TODO: Use the websocket to query the online status instead
/*int64_t Client::UpdateConnectionStatus()
{
    Client& client = Client::GetInstance();
    try
    {
        URI endpoint = client.GetServerGameURL() / "Account" / "status";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        web::http::http_response response = httpClient.request(request).get();
        bool status = (response.status_code() == web::http::status_codes::OK);
        if (client._online != status)
        {
            client._online = status;
            client.UpdateLeagueInfoText();
        }
    }
    catch (const std::exception& ex)
    {
        if (client._online == true)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to query server status: % %", client._online, ex.what());
            client._online = false;
            client.UpdateLeagueInfoText();
        }
    }
    return 10000;
}*/

/*bool Client::Initialize()
{

    //if (!IsOfflineMode())
    {
        // TODO: Move these to the main DLL code
        // Initialize the server sync module
        //ServerSync::Initialize();

        // Initialize the chat client
        //ChatClient::GetInstance();
        //EngineAPI::UI::ChatWindow::GetInstance();

        // Initialize threads to handle refreshing the server token/connection status
        //ThreadManager::CreateThread("refresh_token", 1000, 900000, &Client::UpdateRefreshToken);
        //ThreadManager::CreateThread("connection_status", 1000, 0, &Client::UpdateConnectionStatus);
    }

    // Initialize the death recap module
    //DeathRecap::Initialize();

    //CreatePlayMenu(GetSeasonName());

    return true;
}*/