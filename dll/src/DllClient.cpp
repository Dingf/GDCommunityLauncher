#include <filesystem>
#include <Windows.h>
#include <minizip/unzip.h>
#include "GameAPI/GameFolder.h"
#include "ContextManager.h"
#include "EventManager.h"
#include "DllClient.h"
#include "ChatHandler.h"
#include "ServerHandler.h"
#include "HTTP.h"
#include "Log.h"

DllClient::DllClient() : _activeSeason(nullptr)
{
    ReadDataFromPipe();

    if (!IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,   OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_PRE_SHUTDOWN, OnPreShutdownEvent);
    }
}

DllClient* DllClient::GetInstance()
{
    static DllClient instance;
    return &instance;
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

const SeasonInfo* DllClient::GetSeasonByType(SeasonType type)
{
    for (size_t i = 0; i < _seasons.size(); ++i)
    {
        SeasonInfo& season = _seasons[i];
        if (season._seasonType == type)
        {
            return &season;
        }
    }
    return nullptr;
}

const SeasonInfo* DllClient::GetSeasonByType(bool hardcore)
{
    SeasonType seasonType = hardcore ? SEASON_TYPE_HC_SSF : SEASON_TYPE_SC_TRADE;
    return GetSeasonByType(seasonType);
}

void DllClient::SetActiveSeason(uint32_t seasonID)
{
    _activeSeason = nullptr;
    if (seasonID != 0)
    {
        for (size_t i = 0; i < _seasons.size(); ++i)
        {
            SeasonInfo& season = _seasons[i];
            if (season._seasonID == seasonID)
            {
                _activeSeason = &season;
                break;
            }
        }
    }
}

void DllClient::RefreshAuthToken()
{
    // Attempt to refresh the auth token every 15mins
    // This will automatically update the auth token in the server/chat websockets as well
    _refreshTimer->expires_after(std::chrono::milliseconds(900000));
    _refreshTimer->async_wait([this](const boost::system::error_code& ec)
    {
        try
        {
            if (!ec)
            {
                HTTPRequest request(HTTP_METHOD_POST, "/Account/login");
                request.SetBody({
                    { "username", GetUsername() },
                    { "password", GetPassword() },
                });

                HTTPResponse response = request.Send(spClient->GetHostName(), "443");
                if (response.GetStatus() == 200)
                {
                    json responseJSON = json::parse(response.GetBody());
                    json& accessToken = responseJSON.at("access_token");
                    json& refreshToken = responseJSON.at("refresh_token");

                    if (!accessToken.is_null() && !refreshToken.is_null())
                    {
                        _authToken = accessToken.get<std::string>();
                        _refreshToken = refreshToken.get<std::string>();
                    }
                }
                else
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
                }
            }
            else if (ec != asio::error::operation_aborted)
            {
                throw ec;
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to refresh auth token: %", ec.message());
        }
        RefreshAuthToken();
    });
}

void DllClient::OnInitializeEvent()
{
    spClient->_refreshTimer = std::make_unique<boost::asio::steady_timer>(ContextManager::GetIOContext());
    spClient->RefreshAuthToken();
}

void DllClient::OnPreShutdownEvent()
{
    spClient->_refreshTimer->cancel();
}