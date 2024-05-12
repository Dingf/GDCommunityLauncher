#include <future>
#include <cwctype>
#include <cpprest/http_client.h>
#include <signalrclient/hub_connection.h>
#include <signalrclient/hub_connection_builder.h>
#include <signalrclient/signalr_value.h>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatConnection.h"
#include "Client.h"
#include "EventManager.h"
#include "ThreadManager.h"
#include "Configuration.h"
#include "Item.h"
#include "StringConvert.h"
#include "URI.h"
#include "Log.h"

// TODO: The connection state doesn't update properly?

void ChatClient::OnShutdownEvent()
{
    GetInstance().Disconnect();
}

void ChatClient::OnWorldPreLoadEvent(std::string mapName, bool modded)
{
    GetInstance().LoadConfig();
    GetInstance().LoadMutedList();

    // Connect the client at game start; we can't do this when the DLL is loaded due to networking code
    ChatClient& chatClient = GetInstance();
    if ((!chatClient.IsConnected()) && (chatClient.Connect()))
        chatClient.Invoke("GetConnectionId");
}

void ChatClient::OnSetMainPlayerEvent(void* player)
{
    GetInstance().Invoke("Welcome");
}

void ChatClient::OnConnection(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        chatClient._connectionID = value.as_array()[0].as_string();
        GameAPI::AddChatMessage(L"Server", L"Connected to chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatClient::OnReceiveMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = value.as_array();

        std::wstring name = RawToWide(values[0].as_string());
        std::wstring message = RawToWide(values[1].as_string());
        uint8_t type = (uint8_t)values[2].as_double();

        if (chatClient.IsPlayerMuted(name))
            return;

        if (type == EngineAPI::UI::CHAT_TYPE_WHISPER)
            name = L"[From " + name + L"]";

        void* item = nullptr;
        if ((values.size() >= 4) && (values[3].as_string().size() > 0))
        {
            web::json::value itemJSON = web::json::value::parse(values[3].as_string());
            GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo(Item(itemJSON));
            itemInfo._itemID = EngineAPI::CreateObjectID();
            item = GameAPI::CreateItem(itemInfo);
        }

        GameAPI::AddChatMessage(name, message, type, item);
    }
}

void ChatClient::OnServerMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = value.as_array();

        std::wstring message = CharToWide(values[1].as_string());
        GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatClient::OnWelcomeMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = value.as_array();

        const auto& messages = values[0].as_array();
        for (size_t i = 0; i < messages.size(); ++i)
        {
            std::wstring message = CharToWide(messages[i].as_string());
            GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }

        chatClient.DisplayNewTradeNotifications();
    }
}

void ChatClient::OnJoinedChannel(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = value.as_array();

        uint32_t channels = (uint32_t)values[0].as_double();
        if ((channels & 0xF0) != (chatClient._channels & 0xF0))
        {
            chatClient._channels = (chatClient._channels & 0x0F) | (channels & 0xF0);

            std::wstring joinMessage;
            if ((channels & 0xF0) == 0)
                joinMessage = L"Global chat is now OFF.";
            else
                joinMessage = L"Joined global channel " + std::to_wstring((channels & 0xF0) >> 4) + L".";

            GameAPI::AddChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_GLOBAL);
        }
        if ((channels & 0x0F) != (chatClient._channels & 0x0F))
        {
            chatClient._channels = (chatClient._channels & 0xF0) | (channels & 0x0F);

            std::wstring joinMessage;
            if ((channels & 0x0F) == 0)
                joinMessage = L"Trade chat is now OFF.";
            else
                joinMessage = L"Joined trade channel " + std::to_wstring(channels & 0x0F) + L".";

            GameAPI::AddChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_TRADE);
        }

        chatClient.SaveConfig();
    }
}

void ChatClient::OnBanned(const signalr::value& value)
{
    GameAPI::AddChatMessage(L"Server", L"Your account has been banned from chat.", EngineAPI::UI::CHAT_TYPE_NORMAL);
}

ChatClient::ChatClient(URI endpoint) : Connection(endpoint), _channels(0)
{
    Register("Connection", OnConnection);
    Register("ReceiveMessage", OnReceiveMessage);
    Register("SystemMessage", OnServerMessage);
    Register("WelcomeMessage", OnWelcomeMessage);
    Register("JoinedChannel", OnJoinedChannel);
    Register("Banned", OnBanned);

    EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,        &ChatClient::OnShutdownEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,  &ChatClient::OnWorldPreLoadEvent);
    EventManager::Subscribe(GDCL_EVENT_SET_MAIN_PLAYER, &ChatClient::OnSetMainPlayerEvent);
}

ChatClient::~ChatClient()
{
    EventManager::Unsubscribe(GDCL_EVENT_SHUTDOWN,        &ChatClient::OnShutdownEvent);
    EventManager::Unsubscribe(GDCL_EVENT_WORLD_PRE_LOAD,  &ChatClient::OnWorldPreLoadEvent);
    EventManager::Unsubscribe(GDCL_EVENT_SET_MAIN_PLAYER, &ChatClient::OnSetMainPlayerEvent);
}

ChatClient& ChatClient::GetInstance()
{
    Client& client = Client::GetInstance();
    static ChatClient instance(client.GetServerChatURL());
    return instance;
}

uint8_t ChatClient::GetChannel(EngineAPI::UI::ChatType type) const
{
    if (type == EngineAPI::UI::CHAT_TYPE_GLOBAL)
        return (_channels & 0xF0) >> 4;
    else if (type == EngineAPI::UI::CHAT_TYPE_TRADE)
        return (_channels & 0x0F);
    else
        return 0;
}

void ChatClient::SetChannel(EngineAPI::UI::ChatType type, uint32_t channel)
{
    uint32_t oldChannel = _channels;
    uint32_t newChannel = channel;
    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_NORMAL:
            newChannel = (uint8_t)(channel & 0xFF);
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            newChannel = (oldChannel & 0xF0) | (channel & 0x0F);
            break;
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            newChannel = (oldChannel & 0x0F) | ((channel & 0x0F) << 4);
            break;
    }

    Client& client = Client::GetInstance();
    InvokeAsync("JoinChannel", GetConnectionID(), client.GetUsername(), oldChannel, newChannel);
}

void ChatClient::SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message, void* item)
{
    uint32_t channelValue = 0;
    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            channelValue = _channels & 0xF0;
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            channelValue = _channels & 0x0F;
            break;
    }

    std::string itemJSON = "";
    if (item)
    {
        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(item);
        Item item = InfoToItem(itemInfo);
        itemJSON = JSONString(item.ToJSON().serialize());
    }

    InvokeAsync("Send", WideToRaw(name), WideToRaw(message), channelValue, itemJSON);
}

void ChatClient::SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item)
{
    Client& client = Client::GetInstance();
    std::vector<signalr::value> args;
    uint32_t oldChannel = _channels;
    uint32_t newChannel = channel;
    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_NORMAL:
            newChannel = (uint8_t)(channel & 0xFF);
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            newChannel = (oldChannel & 0xF0) | (channel & 0x0F);
            break;
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            newChannel = (oldChannel & 0x0F) | ((channel & 0x0F) << 4);
            break;
    }

    args.push_back(GetConnectionID());
    args.push_back(client.GetUsername());
    args.push_back((double)oldChannel);
    args.push_back((double)newChannel);

    _connection->invoke("JoinChannel", args, [=](const signalr::value& value, std::exception_ptr ex)
    {
        try
        {
            if (ex)
                std::rethrow_exception(ex);

            SendChatMessage(type, name, message, item);
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to join chat channel: %", ex.what());
        }
    });
}

void ChatClient::DisplayNewTradeNotifications()
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerGameURL() / "Trade" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "trade-notifications" / "new";
    endpoint.AddParam("branch", client.GetBranchName());

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                std::wstring responseBody = response.extract_string().get();
                uint32_t newMessages = std::stoi(responseBody);
                if (newMessages > 0)
                {
                    std::wstring message = L"You have " + responseBody + L" new trade notification";
                    if (newMessages > 1)
                        message += L"s";
                    message += L".";
                    GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_TRADE);
                }
                return;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve new trade notifications: %", ex.what());
    }
}

bool ChatClient::MutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    if (_mutedList.count(playerName) == 0)
    {
        _mutedList.insert(playerName);
        return true;
    }
    else
    {
        return false;
    }
}

bool ChatClient::UnmutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    if (_mutedList.count(playerName) > 0)
    {
        _mutedList.erase(playerName);
        return true;
    }
    else
    {
        return false;
    }
}

bool ChatClient::IsPlayerMuted(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    return (_mutedList.count(playerName) > 0);
}

void ChatClient::LoadConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        const Value* channelValue = config.GetValue("Chat", "channel");
        uint32_t channel = (channelValue) ? channelValue->ToInt() : 0;
        SetChannel(channel);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

void ChatClient::SaveConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        config.SetValue("Chat", "channel", (int)_channels);
        config.Save(configPath);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

void ChatClient::LoadMutedList()
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerChatURL() / "chat" / "mute-list";

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get(); 
            web::json::array mutedList = responseBody.as_array();

            for (size_t i = 0; i < mutedList.size(); ++i)
            {
                // Strip quotes and convert to lower-case before adding the player name
                std::wstring playerName = mutedList[i].serialize();
                playerName = std::wstring(playerName.begin() + 1, playerName.end() - 1);
                std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
                _mutedList.insert(playerName);
            }
        }
        else
        {
            throw std::runtime_error("Server responded with status code %" + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve muted list: %", ex.what());
    }
}