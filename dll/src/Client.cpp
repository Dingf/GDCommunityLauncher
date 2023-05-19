#include <filesystem>
#include <cpprest/http_client.h>
#include <Windows.h>
#include <minizip/unzip.h>
#include "Client.h"
#include "ClientHandlers.h"
#include "ChatClient.h"
#include "HookManager.h"
#include "EventManager.h"
#include "ThreadManager.h"
#include "ServerSync.h"
#include "JSONObject.h"
#include "URI.h"
#include "Log.h"
#include "Version.h"

Client& Client::GetInstance()
{
    static Client instance;
    if (!instance.IsValid())
    {
        instance.ReadDataFromPipe();
    }
    return instance;
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

    char* buffer = new char[length + 1];
    if (!ReadFile(pipe, (LPVOID)buffer, length, &bytesRead, NULL) || (bytesRead != length))
    {
        delete[] buffer;
        return false;
    }

    buffer[length] = '\0';
    str = buffer;

    delete[] buffer;
    return true;
}

bool ReadWideStringFromPipe(HANDLE pipe, std::wstring& str)
{
    uint32_t length;

    if (!ReadInt32FromPipe(pipe, length))
        return false;

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
    return true;
}

bool ReadSeasonsFromPipe(HANDLE pipe, std::vector<SeasonInfo>& seasons)
{
    uint32_t count;
    if (!ReadInt32FromPipe(pipe, count))
        return false;

    for (uint32_t i = 0; i < count; ++i)
    {
        SeasonInfo season;

        if (!ReadInt32FromPipe(pipe, season._seasonID) ||
            !ReadInt32FromPipe(pipe, season._seasonType) ||
            !ReadStringFromPipe(pipe, season._modName) ||
            !ReadStringFromPipe(pipe, season._displayName) ||
            !ReadStringFromPipe(pipe, season._participationToken))
            return false;

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

void Client::ReadDataFromPipe()
{
    if (!IsValid())
    {
        HANDLE pipe = GetStdHandle(STD_INPUT_HANDLE);

        uint32_t updateFlag;
        std::string gameURL;
        std::string chatURL;
        std::string branch;

        if (!ReadStringFromPipe(pipe, _data._username) ||
            !ReadStringFromPipe(pipe, _data._password) ||
            !ReadStringFromPipe(pipe, _data._authToken) ||
            !ReadStringFromPipe(pipe, _data._refreshToken) ||
            !ReadStringFromPipe(pipe, gameURL) ||
            !ReadStringFromPipe(pipe, chatURL) ||
            !ReadStringFromPipe(pipe, branch) ||
            !ReadInt32FromPipe(pipe, updateFlag) ||
            !ReadSeasonsFromPipe(pipe, _data._seasons))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read client data from stdin pipe.");
            return;
        }

        CloseHandle(pipe);

        _data._gameURL = gameURL;
        _data._chatURL = chatURL;
        _data._branch = branch;
        _data._updateFlag = (updateFlag != 0);
        if ((_data._updateFlag != 0) && (!ExtractZIPUpdate()))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to update GDCommunityLauncher.exe");
            return;
        }

        if (_data._seasons.size() > 0)
        {
            _data._seasonName = _data._seasons[0]._modName;
            GameAPI::SetRootPrefix(_data._seasonName);
        }

        UpdateLeagueInfoText();
        UpdateVersionInfoText();
    }
}

bool Client::UpdateRefreshToken()
{
    pplx::create_task([]()
    {
        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Account" / "login";
            web::http::client::http_client httpClient((utility::string_t)endpoint);

            web::json::value requestBody;
            requestBody[U("username")] = JSONString(client._data._username);
            requestBody[U("password")] = JSONString(client._data._password);

            web::http::http_request request(web::http::methods::POST);
            request.set_body(requestBody);

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::value authTokenValue = responseBody[U("access_token")];
                web::json::value refreshTokenValue = responseBody[U("refresh_token")];
                if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
                {
                    client._data._authToken = JSONString(authTokenValue.serialize());
                    client._data._refreshToken = JSONString(refreshTokenValue.serialize());
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
    });
    return true;
}

bool Client::UpdateConnectionStatus()
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
            if (status)
                EventManager::Publish(GDCL_EVENT_CONNECT);
            else
                EventManager::Publish(GDCL_EVENT_DISCONNECT);
        }
    }
    catch (const std::exception& ex)
    {
        if (client._online == true)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to query server status: % %", client._online, ex.what());
            client._online = false;
            EventManager::Publish(GDCL_EVENT_DISCONNECT);
        }
    }
    client.UpdateLeagueInfoText();
    return true;
}

void Client::UpdateVersionInfoText()
{
    typedef const char* (__thiscall* GetVersionProto)(void*);

    _versionInfoText.clear();

    GetVersionProto callback = (GetVersionProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    void** engine = EngineAPI::GetEngineHandle();

    if ((callback) && (engine))
    {
        const char* result = callback((void*)*engine);
        _versionInfoText = result;
        _versionInfoText += "\n{^F}GDCL v";
        _versionInfoText += GDCL_VERSION;
        _versionInfoText += " (";
        _versionInfoText += GetUsername();
        _versionInfoText += ")";
    }
}

void Client::UpdateLeagueInfoText()
{
    _leagueInfoText.clear();

    if (_activeSeason)
    {
        _leagueInfoText += L"\n";
        _leagueInfoText += std::wstring(_activeSeason->_displayName.begin(), _activeSeason->_displayName.end());
    }
    _leagueInfoText += L"\n";
    _leagueInfoText += std::wstring(_data._username.begin(), _data._username.end());

    if ((_online) && (_activeSeason))
    {
        if (GameAPI::IsCloudStorageEnabled())
        {
            _leagueInfoText += L" {^Y}(Disable Cloud Saving)";
        }
        else if (EngineAPI::IsMultiplayer())
        {
            _leagueInfoText += L" {^Y}(Multiplayer)";
        }
        else if (!ServerSync::IsClientTrusted())
        {
            _leagueInfoText += L" {^Y}(Desynced ~ Restart Game)";
        }
        else
        {
            if ((_points > 0) && (_rank > 0))
            {
                _leagueInfoText += L" {^L}(Rank ";
                _leagueInfoText += std::to_wstring(_rank);
                _leagueInfoText += L" ~ ";
            }
            else
            {
                _leagueInfoText += L" {^L}(";
            }
            _leagueInfoText += std::to_wstring(_points);
            _leagueInfoText += L" points)";
        }
    }
    else
    {
        _leagueInfoText += L" {^R}(Offline)";
    }
}

void Client::SetActiveSeason(const std::string& modName, bool hardcore)
{
    _activeSeason = nullptr;
    for (size_t i = 0; i < _data._seasons.size(); ++i)
    {
        SeasonInfo& season = _data._seasons[i];
        if ((modName == season._modName) && ((1 + hardcore) == season._seasonType))
        {
            _activeSeason = &season;
            break;
        }
    }
    UpdateLeagueInfoText();
}

void Client::SetActiveCharacter(const std::wstring& name, bool hasToken)
{
    _activeCharacter._name = name;
    _activeCharacter._hasToken = hasToken;
}

void Client::UpdateSeasonStanding()
{
    pplx::create_task([this]()
    {
        try
        {
            URI endpoint = GetServerGameURL() / "Season" / "participant" / std::to_string(GetParticipantID()) / "standing";
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            web::http::http_response response = httpClient.request(request).get();
            switch (response.status_code())
            {
                case web::http::status_codes::OK:
                {
                    web::json::value responseBody = response.extract_json().get();
                    web::json::value pointTotal = responseBody[U("pointTotal")];
                    web::json::value rank = responseBody[U("rank")];

                    _points = pointTotal.as_integer();
                    _rank = rank.as_integer();
                    UpdateLeagueInfoText();
                    break;
                }
                default:
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update season standing: %", ex.what());
        }
    });
}

void Client::CreatePlayMenu()
{
    std::filesystem::path userSavePath = GameAPI::GetUserSaveFolder();
    if (!std::filesystem::is_directory(userSavePath))
        std::filesystem::create_directories(userSavePath);

    std::filesystem::path playMenuPath = userSavePath / "playmenu.cpn";
    if (!std::filesystem::is_regular_file(playMenuPath))
    {
        std::string seasonName = GetSeasonName();
        std::wstring serverName = L"Grim Dawn Server";
        std::string mapName = "maps/world001.map";

        size_t fileSize = 84 + seasonName.size() + (serverName.size() * sizeof(wchar_t) * 2) + (mapName.size() * 2);
        FileWriter writer(fileSize);

        writer.BufferInt32(0xFFFFFFFF);
        writer.BufferInt64(2);
        writer.BufferInt64(1);
        writer.BufferWideString(serverName);
        writer.BufferWideString(serverName);
        writer.BufferString(mapName);
        writer.BufferInt32(4);      // Max players
        writer.BufferInt32(0xC8);   // Level range
        writer.BufferInt64(0);
        writer.BufferInt32(2);
        writer.BufferString(seasonName);
        writer.BufferString(mapName);
        writer.BufferInt64(1);
        writer.BufferInt64(1);
        writer.BufferInt32(0);
        writer.BufferInt32(0xFFFFFFFF);
        writer.WriteToFile(playMenuPath);
    }
}

bool Client::Initialize()
{
    Logger::SetMinimumLogLevel(LOG_LEVEL_DEBUG);

    // Initialize the server sync module
    ServerSync::Initialize();

    // Initialize the chat client in a separate thread
    pplx::task<void> task = pplx::create_task([]()
    {
        ChatClient& chatClient = ChatClient::GetInstance();
    });

    // Initialize the game engine hooks
    if (!HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION, &HandleGetVersion) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER, &HandleRender) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_READ, &HandleDirectRead) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_WRITE, &HandleDirectWrite) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD, &HandleLoadWorld) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_SET_REGION_OF_NOTE, &HandleSetRegionOfNote) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT, &HandleKeyEvent) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT, &HandleMouseEvent) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D, &HandleRenderStyledText2D) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_LUA_INITIALIZE, &HandleLuaInitialize) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_CREATE_SERVER_CONNECTION, &HandleCreateNewConnection) ||
        !HookManager::CreateHook("Engine.dll", EngineAPI::EAPI_NAME_ADD_NETWORK_SERVER, &HandleAddNetworkServer) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GAME_ENGINE_SHUTDOWN, &HandleGameShutdown) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER, &HandleSetMainPlayer) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT, &HandleSetTransferOpen) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA, &HandleSaveNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_LOAD_NEW_FORMAT_DATA, &HandleLoadNewFormatData) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH, &HandleSaveTransferStash) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH, &HandleLoadTransferStash) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN, &HandleBestowToken) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD, &HandleUnloadWorld) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE, &HandleSendChatMessage) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_SYNC_DUNGEON_PROGRESS, &HandleSyncDungeonProgress) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_USE_ITEM_ENCHANTMENT, &HandleUseItemEnchantment) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_CAN_ENCHANT_BE_USED_ON, &HandleCanEnchantBeUsedOn) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_ITEM_DESCRIPTION, &HandleGetItemDescription) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_WEAPON_DESCRIPTION, &HandleGetItemDescriptionWeapon) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_ARMOR_DESCRIPTION, &HandleGetItemDescriptionArmor) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_ROOT_SAVE_PATH, &HandleGetRootSavePath) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_BASE_FOLDER, &HandleGetBaseFolder) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_USER_SAVE_FOLDER, &HandleGetUserSaveFolder) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_FULL_SAVE_FOLDER, &HandleGetFullSaveFolder) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_1, &HandleGetPlayerFolder1) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_2, &HandleGetPlayerFolder2) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_MAP_FOLDER, &HandleGetMapFolder, true) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_DIFFICULTY_FOLDER, &HandleGetDifficultyFolder) ||
        !HookManager::CreateHook("Game.dll", GameAPI::GAPI_NAME_GET_SHARED_SAVE_PATH, &HandleGetSharedSavePath, true))
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to create one or more game hooks.");
        return false;
    }

    // Initialize threads to handle refreshing the server token/connection status
    ThreadManager::CreatePeriodicThread("refresh_token", 60000, 900000, 0, &Client::UpdateRefreshToken);
    ThreadManager::CreatePeriodicThread("connection_status", 10000, 10000, 0, &Client::UpdateConnectionStatus);

    CreatePlayMenu();
    UpdateVersionInfoText();

    return true;
}

void Client::Cleanup()
{
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_GET_VERSION);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_READ);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_WRITE);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_SET_REGION_OF_NOTE);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    HookManager::DeleteHook("Engine.dll", EngineAPI::EAPI_NAME_LUA_INITIALIZE);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GAME_ENGINE_SHUTDOWN);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_LOAD_NEW_FORMAT_DATA);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_SYNC_DUNGEON_PROGRESS);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_USE_ITEM_ENCHANTMENT);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_CAN_ENCHANT_BE_USED_ON);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_ITEM_DESCRIPTION);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_WEAPON_DESCRIPTION);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_ARMOR_DESCRIPTION);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_ROOT_SAVE_PATH);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_BASE_FOLDER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_USER_SAVE_FOLDER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_FULL_SAVE_FOLDER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_1);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_PLAYER_FOLDER_2);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_DIFFICULTY_FOLDER);
    HookManager::DeleteHook("Game.dll", GameAPI::GAPI_NAME_GET_SHARED_SAVE_PATH);
}