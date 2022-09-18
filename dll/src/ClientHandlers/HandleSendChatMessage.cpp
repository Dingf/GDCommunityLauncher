#include <unordered_map>
#include <regex>
#include <cwctype>
#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ChatClient.h"
#include "Character.h"
#include "JSONObject.h"
#include "URI.h"
#include "Log.h"

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    GameAPI::SendChatMessage(L"/g, /global[CHANNEL] [ON/OFF]", L"Sends a message to the current global chat channel. If no arguments are specified, displays the current global chat channel.\n    [CHANNEL] - Switches the current global chat channel.\n    [ON/OFF] - Enables or disables global chat.\n ", 0);
    GameAPI::SendChatMessage(L"/tr, /trade[CHANNEL] [ON/OFF]", L"Sends a message to the current trade chat channel. If no arguments are specified, displays the current trade chat channel.\n    [CHANNEL] - Switches the current trade chat channel.\n    [ON/OFF] - Enables or disables trade chat.\n ", 0);
    GameAPI::SendChatMessage(L"/o, /online", L"Displays the number of current users online.\n ", 0);
    GameAPI::SendChatMessage(L"/h, /help", L"Displays this help message. ", 0);
    return false;
}

bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/g ");

    type = EngineAPI::UI::CHAT_TYPE_GLOBAL;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
        return false;
    }
    else if (arg == L"off")
    {
        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, 0);
        return false;
    }
    else if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if (message.empty())
            {
                chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                chatClient.SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel, name, message);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in global channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient.SendChatMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, name, message);
            return false;
        }
    }
    else
    {
        name = L"Server";
        message = L"Global chat is currently disabled. You can enable it by typing /global ON.";
    }
    return true;
}

bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/tr ");

    type = EngineAPI::UI::CHAT_TYPE_TRADE;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
        return false;
    }
    else if (arg == L"off")
    {
        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, 0);
        return false;
    }
    else if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if (message.empty())
            {
                chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                chatClient.SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_TRADE, channel, name, message);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in trade channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient.SendChatMessage(EngineAPI::UI::CHAT_TYPE_TRADE, name, message);
            return false;
        }
    }
    else
    {
        name = L"Server";
        message = L"Trade chat is currently disabled. You can enable it by typing /trade ON.";
    }
    return true;
}

bool HandleChatOnlineCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = URI(client.GetHostName()) / "chat" / "chat" / "connected-clients";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::array usersArray = responseBody.as_array();

            std::wstring message = L"There are " + std::to_wstring(usersArray.size()) + L" users currently online.";
            GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve online users: %", ex.what());
    }

    return false;
}

typedef bool(*ChatCommandHandler)(std::wstring&, std::wstring&, uint32_t&, uint8_t&);
const std::unordered_map<std::wstring, ChatCommandHandler> chatCommandLookup =
{
    { L"help",     HandleChatHelpCommand },
    { L"h",        HandleChatHelpCommand },
    { L"global",   HandleChatGlobalCommand },
    { L"g",        HandleChatGlobalCommand },
    { L"trade",    HandleChatTradeCommand },
    { L"tr",       HandleChatTradeCommand },
    { L"o",        HandleChatOnlineCommand },
    { L"online",   HandleChatOnlineCommand },
};

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t unk1)
{
    typedef void(__thiscall* HandleSendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    HandleSendChatMessageProto callback = (HandleSendChatMessageProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        std::wstring realName = name;
        std::wstring realMessage = message;

        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            chatWindow.SetChatPrefix({});

            std::wsmatch match;
            std::wregex commandRegex(L"^\\/([A-Za-z_]+)(\\d*)\\s*(.*)$");
            if (std::regex_match(message, match, commandRegex))
            {
                std::wstring command = match.str(1);
                std::transform(command.begin(), command.end(), command.begin(), std::towlower);
                uint32_t channel = 0;
                realMessage = match.str(3);

                try
                {
                    channel = std::stoi(match.str(2));
                }
                catch (std::exception&) {}

                if (chatCommandLookup.count(command) > 0)
                {
                    ChatCommandHandler handler = chatCommandLookup.at(command);
                    if (!handler(realName, realMessage, channel, type))
                        return;
                }
            }
        }

        callback(_this, realName, realMessage, type, targets, unk1);
    }
}
