#include <filesystem>
#include "LauncherClient.h"
#include "Log.h"
#include "dll/include/SeasonClient.h"

LauncherClient& LauncherClient::GetInstance()
{
    static LauncherClient instance;
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

bool LauncherClient::WriteDataToPipe(HANDLE pipe) const
{
    if (!WriteStringToPipe(pipe, _username) ||
        !WriteStringToPipe(pipe, _password) ||
        !WriteStringToPipe(pipe, _authToken) ||
        !WriteStringToPipe(pipe, _refreshToken) ||
        !WriteStringToPipe(pipe, _seasonName) ||
        !WriteStringToPipe(pipe, _gameURL) ||
        !WriteStringToPipe(pipe, _chatURL) ||
        !WriteInt32ToPipe(pipe, _branch) ||
        !WriteByteToPipe(pipe, (uint8_t)_info._hasUpdate) ||
        !WriteSeasonsToPipe(pipe, spClient->GetSeasonList())) // TODO correct refactor?
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write client data to the stdin pipe.");
        return false;
    }

    CloseHandle(pipe);
    return TRUE;
}

/* TODO?
void LauncherClient::CreateConnection(const std::string& url)
{
    _gameURL = url;
    _connection = std::make_unique<Connection>(_gameURL);
    _connection->Register("Login", Client::OnLogin);
    _connection->Register("GetChatUrl", Client::OnGetChatUrl);
    _connection->Register("GetLatestSeasonName", Client::OnGetSeasonName);
    _connection->Register("GetLatestSeason", Client::OnGetSeasonData);
    _connection->Register("GetLauncherFile", Client::OnGetLauncherVersion);
    _connection->Register("GetLeagueFiles", Client::OnGetSeasonFiles);
}*/

void LauncherClient::OnLogin(const signalr::value& value)
{
    LauncherClient& client = LauncherClient::GetInstance();
    if (value.is_array())
    {
        try
        {
            json responseJSON = json::parse(value.as_string());
            client._authToken = responseJSON.at("access_token").get<std::string>();
            client._refreshToken = responseJSON.at("refresh_token").get<std::string>();
            if (responseJSON.count("role") > 0)
            {
                client._role = responseJSON.at("role").get<std::string>();
            }
            // TODO Test if above code refactors the below
            /*web::json::value loginJSON = web::json::value::parse(value.as_array()[0].as_string());
            client._authToken = JSONString(loginJSON.at(U("access_token")).serialize());
            client._refreshToken = JSONString(loginJSON.at(U("refresh_token")).serialize());

            if (loginJSON.has_string_field(U("role")))
                client._role = JSONString(loginJSON.at(U("role")).serialize());*/
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to login to server: %", ex.what());
        }
    }
}

void LauncherClient::OnGetChatUrl(const signalr::value& value)
{
    LauncherClient& client = LauncherClient::GetInstance();
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

void LauncherClient::OnGetSeasonName(const signalr::value& value)
{
    LauncherClient& client = LauncherClient::GetInstance();
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

void LauncherClient::OnGetSeasonData(const signalr::value& value)
{
    /* TODO Refactor to GetSeasons ws
    SeasonClient* client = SeasonClient::GetInstance();
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
                seasonInfo._seasonType = static_cast<SeasonType>(seasonJSON.at(U("SeasonTypeId")).as_integer());

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

                //TODO client->_seasons.push_back(seasonInfo);
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season data from server: %", ex.what());
            }
        }
    }*/
}

void LauncherClient::OnGetLauncherVersion(const signalr::value& value)
{
    LauncherClient& client = LauncherClient::GetInstance();
    if (value.is_array())
    {
        try
        {
            json responseJSON = json::parse(value.as_string());
            client._info._size = responseJSON.at("FileSize").get<uint32_t>();
            client._info._filename = responseJSON.at("FileName").get<std::string>();
            client._info._checksum = responseJSON.at("Checksum").get<std::string>();
            client._info._version = responseJSON.at("Version").get<std::string>();
            client._info._downloadURL = responseJSON.at("DownloadUrl").get<std::string>();
            client._info._hasUpdate = (client._info._version != GDCL_VERSION);
            // TODO Test if above code refactors the below
            /*
            web::json::value launcherJSON = web::json::value::parse(value.as_array()[0].as_string());
            client._info._size = launcherJSON.at(U("FileSize")).as_integer();
            client._info._filename = JSONString(launcherJSON.at(U("FileName")).serialize());
            client._info._checksum = JSONString(launcherJSON.at(U("Checksum")).serialize());
            client._info._version = JSONString(launcherJSON.at(U("Version")).serialize());
            client._info._downloadURL = JSONString(launcherJSON.at(U("DownloadUrl")).serialize());
            client._info._hasUpdate = (client._launcher._version != GDCL_VERSION);*/
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve launcher data from server: %", ex.what());
        }
    }
}

void LauncherClient::OnGetSeasonFiles(const signalr::value& value)
{
    /* TODO Refactor 
    LauncherClient& client = LauncherClient::GetInstance();
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

        if (client.HasUpdate())
        {
            std::filesystem::path filenamePath = std::filesystem::current_path() / client._info._filename;
            client._downloadList[filenamePath.wstring()] = client._info._downloadURL;
        }
    }*/
}