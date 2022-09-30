#include <future>
#include <signalrclient/connection.h>
#include <signalrclient/hub_connection.h>
#include <signalrclient/hub_connection_builder.h>
#include <signalrclient/signalr_value.h>
#include "EngineAPI/UI/ChatWindow.h"
#include "GameAPI.h"
#include "ChatClient.h"
#include "Client.h"
#include "EventManager.h"
#include "Configuration.h"
#include "URI.h"
#include "Log.h"

class ChatClientLogger : public signalr::log_writer
{
    // Inherited via log_writer
    virtual void __cdecl write(const std::string& entry) override
    {
        Logger::LogMessage(LOG_LEVEL_DEBUG, entry);
    }
};

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

void ChatClient::OnConnection(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        chatClient._connectionID = m.as_array()[0].as_string();

        EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
        if (chatWindow.IsInitialized())
            GameAPI::SendChatMessage(L"Server", L"Connected to chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);

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

        GameAPI::SendChatMessage(name, message, type);
    }
}

void ChatClient::OnServerMessage(const signalr::value& m)
{
    if (!m.is_null())
    {
        ChatClient& chatClient = ChatClient::GetInstance();
        const auto& values = m.as_array();

        std::wstring message = CharToWide(values[1].as_string());
        GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
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
            GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }
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

            GameAPI::SendChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_GLOBAL);
        }
        if ((channels & 0x0F) != (chatClient._channels & 0x0F))
        {
            chatClient._channels = (chatClient._channels & 0xF0) | (channels & 0x0F);

            std::wstring joinMessage;
            if ((channels & 0x0F) == 0)
                joinMessage = L"Trade chat is now OFF.";
            else
                joinMessage = L"Joined trade channel " + std::to_wstring(channels & 0x0F) + L".";

            GameAPI::SendChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_TRADE);
        }

        chatClient.SaveConfig();
    }
}

void ChatClient::OnBanned(const signalr::value& m)
{
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    if (chatWindow.IsInitialized())
        GameAPI::SendChatMessage(L"Server", L"Your account has been banned from chat.", EngineAPI::UI::CHAT_TYPE_NORMAL);
}

ChatClient::ChatClient()
{
    Client& client = Client::GetInstance();
    URI endpoint = URI(client.GetHostName()) / "chat";

    _connection = std::make_unique<signalr::hub_connection>(signalr::hub_connection_builder::create(endpoint).skip_negotiation().with_logging(std::make_shared<ChatClientLogger>(), signalr::trace_level::verbose).build());
    _connection->on("Connection", OnConnection);
    _connection->on("ReceiveMessage", OnReceiveMessage);
    _connection->on("SystemMessage", OnServerMessage);
    _connection->on("WelcomeMessage", OnWelcomeMessage);
    _connection->on("JoinedChannel", OnJoinedChannel);
    _connection->on("Banned", OnBanned);

    EventManager::Subscribe(GDCL_EVENT_CONNECT, &ChatClient::OnConnectEvent);
    EventManager::Subscribe(GDCL_EVENT_DISCONNECT, &ChatClient::OnDisconnectEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_LOAD, &ChatClient::OnWorldLoadEvent);

    _channels = 0;
}

ChatClient::~ChatClient()
{
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

uint8_t ChatClient::GetCurrentChatChannel(EngineAPI::UI::ChatType type) const
{
    if (type == EngineAPI::UI::CHAT_TYPE_GLOBAL)
        return (_channels & 0xF0) >> 4;
    else if (type == EngineAPI::UI::CHAT_TYPE_TRADE)
        return (_channels & 0x0F);
    else
        return 0;
}

void ChatClient::SetCurrentChatChannel(EngineAPI::UI::ChatType type, uint32_t channel)
{
    if (channel <= EngineAPI::UI::CHAT_CHANNEL_MAX)
    {
        uint8_t newChannels;
        if (type == EngineAPI::UI::CHAT_TYPE_GLOBAL)
            newChannels = (_channels & 0x0F) | ((channel & 0x0F) << 4);
        else if (type == EngineAPI::UI::CHAT_TYPE_TRADE)
            newChannels = (_channels & 0xF0) | (channel & 0x0F);

        Client& client = Client::GetInstance();
        std::vector<signalr::value> args;
        args.push_back(_connectionID);
        args.push_back(client.GetUsername());
        args.push_back((double)_channels);
        args.push_back((double)newChannels);

        _connection->invoke("JoinChannel", args, [](const signalr::value& value, std::exception_ptr ex)
        {
            LogExceptionPointer(ex, "Failed to join chat channel: %");
        });
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Tried to set chat channel=% for chat=%, which is outside the maximum value range", channel, type);
    }
}

void ChatClient::SetCurrentChatChannel(uint32_t channel)
{
    uint8_t newChannels = (uint8_t)(channel & 0xFF);

    Client& client = Client::GetInstance();
    std::vector<signalr::value> args;
    args.push_back(_connectionID);
    args.push_back(client.GetUsername());
    args.push_back((double)_channels);
    args.push_back((double)newChannels);

    _connection->invoke("JoinChannel", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to join chat channel: %");
    });
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
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    if (chatWindow.IsInitialized())
        GameAPI::SendChatMessage(L"Server", L"Disconnected from chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);

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
}

void ChatClient::SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message)
{
    std::vector<signalr::value> args;
    args.push_back(WideToRaw(name));
    args.push_back(WideToRaw(message));
    
    switch (type)
    {
        case EngineAPI::UI::CHAT_TYPE_GLOBAL:
            args.push_back((double)(_channels & 0xF0));
            break;
        case EngineAPI::UI::CHAT_TYPE_TRADE:
            args.push_back((double)(_channels & 0x0F));
            break;
        default:
            args.push_back((double)0);
            break;
    }

    _connection->invoke("Send", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to send chat message: %");
    });
}

void ChatClient::SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message)
{
    if (channel <= EngineAPI::UI::CHAT_CHANNEL_MAX)
    {
        uint8_t newChannels;
        if (type == EngineAPI::UI::CHAT_TYPE_GLOBAL)
            newChannels = (_channels & 0x0F) | ((channel & 0x0F) << 4);
        else if (type == EngineAPI::UI::CHAT_TYPE_TRADE)
            newChannels = (_channels & 0xF0) | (channel & 0x0F);

        Client& client = Client::GetInstance();
        std::vector<signalr::value> joinArgs;
        joinArgs.push_back(_connectionID);
        joinArgs.push_back(client.GetUsername());
        joinArgs.push_back((double)_channels);
        joinArgs.push_back((double)newChannels);

        _connection->invoke("JoinChannel", joinArgs, [type, name, message, newChannels](const signalr::value& value, std::exception_ptr ex)
        {
            if (!LogExceptionPointer(ex, "Failed to join chat channel: %"))
            {
                ChatClient& chatClient = ChatClient::GetInstance();
                std::vector<signalr::value> sendArgs;
                sendArgs.push_back(WideToRaw(name));
                sendArgs.push_back(WideToRaw(message));

                switch (type)
                {
                    case EngineAPI::UI::CHAT_TYPE_GLOBAL:
                        sendArgs.push_back((double)(newChannels & 0xF0));
                        break;
                    case EngineAPI::UI::CHAT_TYPE_TRADE:
                        sendArgs.push_back((double)(newChannels & 0x0F));
                        break;
                    default:
                        sendArgs.push_back((double)0);
                        break;
                }

                chatClient._connection->invoke("Send", sendArgs, [](const signalr::value& value, std::exception_ptr ex)
                {
                    LogExceptionPointer(ex, "Failed to send chat message: %");
                });
            }
        });
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Tried to set chat channel=% for chat=%, which is outside the maximum value range", channel, type);
    }
}

void ChatClient::DisplayWelcomeMessage()
{
    std::vector<signalr::value> args;
    _connection->invoke("Welcome", args, [](const signalr::value& value, std::exception_ptr ex)
    {
        LogExceptionPointer(ex, "Failed to retrieve welcome message: %");
    });
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
        SetCurrentChatChannel(channel);
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