#include <filesystem>
#include "Client.h"
#include "Log.h"

Client& Client::GetInstance()
{
    static Client instance;
    return instance;
}

bool WriteByteToPipe(HANDLE pipe, uint8_t value)
{
    DWORD bytesWritten;
    return (WriteFile(pipe, &value, 1, &bytesWritten, NULL) && (bytesWritten == 1));
}

bool WriteInt16ToPipe(HANDLE pipe, wchar_t value)
{
    DWORD bytesWritten;
    uint8_t buffer[2];

    buffer[0] = value & 0x00FF;
    buffer[1] = (value & 0xFF00) >> 8;

    return (WriteFile(pipe, buffer, 2, &bytesWritten, NULL) && (bytesWritten == 2));
}

bool WriteInt32ToPipe(HANDLE pipe, uint32_t value)
{
    DWORD bytesWritten;
    uint8_t buffer[4];

    buffer[0] =  value & 0x000000FF;
    buffer[1] = (value & 0x0000FF00) >> 8;
    buffer[2] = (value & 0x00FF0000) >> 16;
    buffer[3] = (value & 0xFF000000) >> 24;

    return (WriteFile(pipe, buffer, 4, &bytesWritten, NULL) && (bytesWritten == 4));
}

bool WriteStringToPipe(HANDLE pipe, const std::string& str)
{
    DWORD bytesWritten;
    uint32_t length = (uint32_t)str.length();

    if (!WriteInt32ToPipe(pipe, length))
        return false;

    if ((length > 0) && (!WriteFile(pipe, str.c_str(), length, &bytesWritten, NULL) || (bytesWritten != length)))
        return false;

    return true;
}

bool WriteWideStringToPipe(HANDLE pipe, const std::wstring& str)
{
    uint32_t length = (uint32_t)str.length();
    if (!WriteInt32ToPipe(pipe, length))
        return false;

    for (size_t i = 0; i < length; ++i)
    {
        if (!WriteInt16ToPipe(pipe, str[i]))
            return false;
    }
    return true;
}

bool WriteSeasonsToPipe(HANDLE pipe, const std::vector<SeasonInfo>& seasons)
{
    uint32_t length = (uint32_t)seasons.size();
    if (!WriteInt32ToPipe(pipe, length))
        return false;

    for (size_t i = 0; i < length; ++i)
    {
        if (!WriteInt32ToPipe(pipe, seasons[i]._seasonID) ||
            !WriteInt32ToPipe(pipe, seasons[i]._seasonType) ||
            !WriteStringToPipe(pipe, seasons[i]._displayName) ||
            !WriteStringToPipe(pipe, seasons[i]._participationToken))
            return false;
    }
    return true;
}

bool Client::WriteDataToPipe(HANDLE pipe) const
{
    if (!WriteStringToPipe(pipe, _username) ||
        !WriteStringToPipe(pipe, _password) ||
        !WriteStringToPipe(pipe, _authToken) ||
        !WriteStringToPipe(pipe, _refreshToken) ||
        !WriteStringToPipe(pipe, _seasonName) ||
        !WriteStringToPipe(pipe, _gameURL) ||
        !WriteStringToPipe(pipe, _chatURL) ||
        !WriteInt32ToPipe(pipe, _branch) ||
        !WriteByteToPipe(pipe, (uint8_t)_launcher._hasUpdate) ||
        !WriteSeasonsToPipe(pipe, _seasons))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write client data to the stdin pipe.");
        return false;
    }

    CloseHandle(pipe);
    return TRUE;
}

void Client::CreateConnection(const std::string& url)
{
    _gameURL = url;
    _connection = std::make_unique<Connection>(_gameURL);
    _connection->Register("Login", Client::OnLogin);
    _connection->Register("GetChatUrl", Client::OnGetChatUrl);
    _connection->Register("GetLatestSeasonName", Client::OnGetSeasonName);
    _connection->Register("GetLatestSeason", Client::OnGetSeasonData);
    _connection->Register("GetLauncherFile", Client::OnGetLauncherVersion);
    _connection->Register("GetLeagueFiles", Client::OnGetSeasonFiles);
}

void Client::OnLogin(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        try
        {
            web::json::value loginJSON = web::json::value::parse(value.as_array()[0].as_string());
            client._authToken = JSONString(loginJSON.at(U("access_token")).serialize());
            client._refreshToken = JSONString(loginJSON.at(U("refresh_token")).serialize());

            if (loginJSON.has_string_field(U("role")))
                client._role = JSONString(loginJSON.at(U("role")).serialize());
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to login to server: %", ex.what());
        }
    }
}

void Client::OnGetChatUrl(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        try
        {
            client._chatURL = value.as_array()[0].as_string();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve chat URL: %", ex.what());
        }
    }
}

void Client::OnGetSeasonName(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        try
        {
            client._seasonName = value.as_array()[0].as_string();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season name: %", ex.what());
        }
    }
}

void Client::OnGetSeasonData(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        std::vector<signalr::value> seasonList = value.as_array()[0].as_array();
        for (size_t i = 0; i < seasonList.size(); ++i)
        {
            try
            {
                web::json::value seasonJSON = web::json::value::parse(seasonList[i].as_string());

                SeasonInfo seasonInfo;
                seasonInfo._seasonID = seasonJSON.at(U("SeasonId")).as_integer();
                seasonInfo._seasonType = seasonJSON.at(U("SeasonTypeId")).as_integer();

                std::string modName = JSONString(seasonJSON.at(U("ModName")).serialize());
                std::string displayName = JSONString(seasonJSON.at(U("DisplayName")).serialize());
                std::string participationToken = JSONString(seasonJSON.at(U("ParticipationTag")).serialize());

                // Trim quotes from serializing the string
                if ((modName.front() == '"') && (modName.back() == '"'))
                    modName = std::string(modName.begin() + 1, modName.end() - 1);
                if ((displayName.front() == '"') && (displayName.back() == '"'))
                    displayName = std::string(displayName.begin() + 1, displayName.end() - 1);
                if ((participationToken.front() == '"') && (participationToken.back() == '"'))
                    participationToken = std::string(participationToken.begin() + 1, participationToken.end() - 1);

                for (char& c : participationToken)
                    c = std::tolower(c);

                seasonInfo._displayName = displayName;
                seasonInfo._participationToken = participationToken;

                client._seasons.push_back(seasonInfo);
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season data from server: %", ex.what());
            }
        }
    }
}

void Client::OnGetLauncherVersion(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        try
        {
            web::json::value launcherJSON = web::json::value::parse(value.as_array()[0].as_string());
            client._launcher._size = launcherJSON.at(U("FileSize")).as_integer();
            client._launcher._filename = JSONString(launcherJSON.at(U("FileName")).serialize());
            client._launcher._checksum = JSONString(launcherJSON.at(U("Checksum")).serialize());
            client._launcher._version = JSONString(launcherJSON.at(U("Version")).serialize());
            client._launcher._downloadURL = JSONString(launcherJSON.at(U("DownloadUrl")).serialize());
            client._launcher._hasUpdate = (client._launcher._version != GDCL_VERSION);
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve launcher data from server: %", ex.what());
        }
    }
}

void Client::OnGetSeasonFiles(const signalr::value& value)
{
    Client& client = Client::GetInstance();
    if (value.is_array())
    {
        std::string seasonName = client.GetSeasonName();
        client._downloadList.clear();

        std::vector<signalr::value> fileList = value.as_array()[0].as_array();
        for (size_t i = 0; i < fileList.size(); ++i)
        {
            try
            {
                web::json::value fileJSON = web::json::value::parse(fileList[i].as_string());
                std::string filename = JSONString(fileJSON.at(U("FileName")).serialize());
                std::string downloadURL = JSONString(fileJSON.at(U("DownloadUrl")).serialize());
                uintmax_t fileSize = std::stoull(JSONString(fileJSON.at(U("FileSize")).serialize()));

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
                    client._downloadList[filenamePath.wstring()] = downloadURL;
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve file list from server: %", ex.what());
            }
        }

        if (client.HasLauncherUpdate())
        {
            std::filesystem::path filenamePath = std::filesystem::current_path() / client._launcher._filename;
            client._downloadList[filenamePath.wstring()] = client._launcher._downloadURL;
        }
    }
}