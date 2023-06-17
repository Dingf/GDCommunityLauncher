#include <future>
#include <cwctype>
#include <cpprest/http_client.h>
#include <signalrclient/hub_connection.h>
#include <signalrclient/hub_connection_builder.h>
#include <signalrclient/signalr_value.h>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatClient.h"
#include "Client.h"
#include "EventManager.h"
#include "Configuration.h"
#include "Item.h"
#include "URI.h"
#include "Log.h"

class ChatClientLogger : public signalr::log_writer
{
    // Inherited via log_writer
    virtual void __cdecl write(const std::string& entry) override
    {
        Logger::LogMessage(LOG_LEVEL_INFO, entry);
    }
};

GameAPI::ItemReplicaInfo ItemToInfo(const Item& item)
{
    GameAPI::ItemReplicaInfo info;
    info._itemName = item._itemName;
    info._itemPrefix = item._itemPrefix;
    info._itemSuffix = item._itemSuffix;
    info._itemModifier = item._itemModifier;
    info._itemIllusion = item._itemIllusion;
    info._itemComponent = item._itemComponent;
    info._itemCompletion = item._itemCompletion;
    info._itemAugment = item._itemAugment;
    info._itemSeed = item._itemSeed;
    info._itemComponentSeed = item._itemComponentSeed;
    info._unk3 = item._itemUnk1;
    info._itemAugmentSeed = item._itemAugmentSeed;
    info._unk4 = item._itemUnk2;
    info._itemStackCount = item._itemStackCount;
    return info;
}

Item InfoToItem(const GameAPI::ItemReplicaInfo& info)
{
    Item item;
    item._itemName = info._itemName;
    item._itemPrefix = info._itemPrefix;
    item._itemSuffix = info._itemSuffix;
    item._itemModifier = info._itemModifier;
    item._itemIllusion = info._itemIllusion;
    item._itemComponent = info._itemComponent;
    item._itemCompletion = info._itemCompletion;
    item._itemAugment = info._itemAugment;
    item._itemSeed = info._itemSeed;
    item._itemComponentSeed = info._itemComponentSeed;
    item._itemUnk1 = info._unk3;
    item._itemAugmentSeed = info._itemAugmentSeed;
    item._itemUnk2 = info._unk4;
    item._itemStackCount = info._itemStackCount;
    return item;
}

std::wstring RawToWide(const std::string& str)
{
    std::wstring result;
    for (size_t i = 0; (i + 1) < str.size(); i += 2)
    {
        result.push_back((((wchar_t)str[i]) << 8) | ((wchar_t)str[i+1]));
    }
    return result;
}

std::string WideToRaw(const std::wstring& str)
{
    std::string result;
    for (size_t i = 0; i < str.size(); ++i)
    {
        result.push_back((char)((str[i] >> 8) & 0xFF));
        result.push_back((char)(str[i] & 0xFF));
    }
    return result;
}

std::wstring CharToWide(const std::string& str)
{
    std::wstring result;
    for (size_t i = 0; i < str.size(); ++i)
    {
        result.push_back((wchar_t)str[i]);
    }
    return result;
}

inline bool LogExceptionPointer(const std::exception_ptr& exception, const std::string& message)
{
    if (exception)
    {
        try
        {
            std::rethrow_exception(exception);
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, message, ex.what());
        }
        return true;
    }
    return false;
}

void ChatClient::OnConnectEvent(void* data)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    chatClient._connection->start([](std::exception_ptr ex)
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        if (!LogExceptionPointer(ex, "Failed to connect chat client: %"))
        {
            chatClient._connection->invoke("GetConnectionId", std::vector<signalr::value>(), [](const signalr::value& value, std::exception_ptr ex2)
            {
                LogExceptionPointer(ex2, "Failed to retrieve connection ID: %");
            });
        }
    });
}

void ChatClient::OnDisconnectEvent(void* data)
{
    Client& client = Client::GetInstance();
    if (client.GetActiveSeason() != nullptr)
    {
        GameAPI::AddChatMessage(L"Server", L"Disconnected from chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);
    }

    ChatClient& chatClient = ChatClient::GetInstance();
    chatClient._connection->stop([](std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to disconnect chat client: %");
    });
}

void ChatClient::OnWorldLoadEvent(void* data)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    chatClient.LoadConfig();
    chatClient.LoadMutedList();
}

void ChatClient::OnConnection(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        chatClient._connectionID = m.as_array()[0].as_string();
        GameAPI::AddChatMessage(L"Server", L"Connected to chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatClient::OnReceiveMessage(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = m.as_array();

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
            GameAPI::ItemReplicaInfo itemInfo = ItemToInfo(Item(itemJSON));
            item = GameAPI::CreateItem(itemInfo);
        }

        GameAPI::AddChatMessage(name, message, type, item);
    }
}

void ChatClient::OnServerMessage(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = m.as_array();

        std::wstring message = CharToWide(values[1].as_string());
        GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatClient::OnWelcomeMessage(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = m.as_array();

        const auto& messages = values[0].as_array();
        for (size_t i = 0; i < messages.size(); ++i)
        {
            std::wstring message = CharToWide(messages[i].as_string());
            GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }

        chatClient.DisplayNewTradeNotifications();
    }
}

void ChatClient::OnJoinedChannel(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = m.as_array();

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

void ChatClient::OnBanned(const signalr::value& m)
{
    GameAPI::AddChatMessage(L"Server", L"Your account has been banned from chat.", EngineAPI::UI::CHAT_TYPE_NORMAL);
}

ChatClient::ChatClient()
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerChatURL();

    _connection = std::make_unique<signalr::hub_connection>(signalr::hub_connection_builder::create(endpoint).skip_negotiation().with_logging(std::make_shared<ChatClientLogger>(), signalr::trace_level::verbose).build());
    _connection->on("Connection", OnConnection);
    _connection->on("ReceiveMessage", OnReceiveMessage);
    _connection->on("SystemMessage", OnServerMessage);
    _connection->on("WelcomeMessage", OnWelcomeMessage);
    _connection->on("JoinedChannel", OnJoinedChannel);
    _connection->on("Banned", OnBanned);

    EventManager::Subscribe(GDCL_EVENT_CONNECT, &ChatClient::OnConnectEvent);
    //EventManager::Subscribe(GDCL_EVENT_DISCONNECT, &ChatClient::OnDisconnectEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD, &ChatClient::OnWorldLoadEvent);

    _channels = 0;
}

ChatClient::~ChatClient()
{
    EventManager::Unsubscribe(GDCL_EVENT_CONNECT, &ChatClient::OnConnectEvent);
    //EventManager::Unsubscribe(GDCL_EVENT_DISCONNECT, &ChatClient::OnDisconnectEvent);
    EventManager::Unsubscribe(GDCL_EVENT_WORLD_PRE_LOAD, &ChatClient::OnWorldLoadEvent);

    _connection->stop([](std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to shut down chat client: %");
    });
}

ChatClient& ChatClient::GetInstance()
{
    static ChatClient instance;
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

std::vector<signalr::value> BuildSetChannelArgs(EngineAPI::UI::ChatType type, uint32_t oldChannel, uint32_t newChannel)
{
    std::vector<signalr::value> args;
    uint32_t newChannelValue = newChannel;
    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_NORMAL:
            newChannelValue = (uint8_t)(newChannel & 0xFF);
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            newChannelValue = (oldChannel & 0xF0) | (newChannel & 0x0F);
            break;
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            newChannelValue = (oldChannel & 0x0F) | ((newChannel & 0x0F) << 4);
            break;
    }

    Client& client = Client::GetInstance();
    ChatClient& chatClient = ChatClient::GetInstance();
    args.push_back(chatClient.GetConnectionID());
    args.push_back(client.GetUsername());
    args.push_back((double)oldChannel);
    args.push_back((double)newChannelValue);

    return args;
}

void ChatClient::SetChannel(EngineAPI::UI::ChatType type, uint32_t channel)
{
    std::vector<signalr::value> args = BuildSetChannelArgs(type, _channels, channel);
    _connection->invoke("JoinChannel", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to join chat channel: %");
    });
}

void ChatClient::SetChannel(uint32_t channel)
{
    std::vector<signalr::value> args = BuildSetChannelArgs(EngineAPI::UI::CHAT_TYPE_NORMAL, _channels, channel);
    _connection->invoke("JoinChannel", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to join chat channel: %");
    });
}

std::vector<signalr::value> BuildSendMessageArgs(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message, uint32_t channel, void* item)
{
    std::vector<signalr::value> args;
    args.push_back(WideToRaw(name));
    args.push_back(WideToRaw(message));

    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            args.push_back((double)(channel & 0xF0));
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            args.push_back((double)(channel & 0x0F));
            break;
        default:
            args.push_back((double)0);
            break;
    }

    if (item != nullptr)
    {
        GameAPI::ItemReplicaInfo itemInfo;
        GameAPI::GetItemReplicaInfo(item, itemInfo);
        Item item = InfoToItem(itemInfo);
        std::string itemJSON = JSONString(item.ToJSON().serialize());
        args.push_back(itemJSON);
    }
    else
    {
        args.push_back("");
    }
    return args;
}

void ChatClient::SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message, void* item)
{
    std::vector<signalr::value> args = BuildSendMessageArgs(type, name, message, _channels, item);
    _connection->invoke("Send", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to send chat message: %");
    });
}

void ChatClient::SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item)
{
    std::vector<signalr::value> joinArgs = BuildSetChannelArgs(type, _channels, channel);
    uint32_t newChannels = (uint32_t)(joinArgs[3].as_double());
    _connection->invoke("JoinChannel", joinArgs, [this, type, name, message, newChannels, item](const signalr::value& value, std::exception_ptr ex)
    {
        if (!LogExceptionPointer(ex, "Failed to join chat channel: %"))
        {
            std::vector<signalr::value> sendArgs = BuildSendMessageArgs(type, name, message, newChannels, item);
            this->_connection->invoke("Send", sendArgs, [](const signalr::value& value, std::exception_ptr ex)
            {
                LogExceptionPointer(ex, "Failed to send chat message: %");
            });
        }
    });
}

void ChatClient::DisplayWelcomeMessage()
{
    std::vector<signalr::value> args;
    _connection->invoke("Welcome", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to retrieve welcome message: %");
    });
}

void ChatClient::DisplayNewTradeNotifications()
{
    Client& client = Client::GetInstance();
    URI endpoint = client.GetServerGameURL() / "Trade" / "participant" / std::to_string(client.GetParticipantID()) / "trade-notifications" / "new";
    endpoint.AddParam("branch", client.GetBranch());

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