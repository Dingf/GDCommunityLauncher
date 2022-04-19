#include <unordered_map>
#include <regex>
#include <cwctype>
#include "ClientHandlers.h"

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    GameAPI::SendChatMessage(L"/g, /global[CHANNEL] [ON/OFF]", L"Sends a message to the current global chat channel.\n    [CHANNEL] - Switches the current global chat channel.\n    [ON/OFF] - Enables or disables global chat.\n ", 0);
    GameAPI::SendChatMessage(L"/tr, /trade[CHANNEL] [ON/OFF]", L"Sends a message to the current trade chat channel.\n    [CHANNEL] - Switches the current trade chat channel.\n    [ON/OFF] - Enables or disables trade chat.\n ", 0);
    GameAPI::SendChatMessage(L"/h, /help", L"Displays this help message.\n ", 0);
    return false;
}

//TODO: Send the global/trade chat messages to the server and also add listeners on the client for incoming messages
bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    Client& client = Client::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/g ");

    type = EngineAPI::UI::CHAT_TYPE_GLOBAL;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
        {
            channel = 1;
            client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, 1);
        }

        name = L"Server";
        message = L"Global chat is now ON. Joined global channel " + std::to_wstring(channel) + L".";
        return true;
    }
    else if (arg == L"off")
    {
        client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, 0);
        name = L"Server";
        message = L"Global chat is now OFF.";
        return true;
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
            client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
            std::wstring joinMessage = L"Joined global channel " + std::to_wstring(channel) + L".";
            if (message.empty())
            {
                name = L"Server";
                message = joinMessage;
                return true;
            }
            else
            {
                GameAPI::SendChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_GLOBAL);
            }
        }
    }

    uint8_t currentChannel = client.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in global channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            // TODO: Send the message to the server here
        }
    }
    else
    {
        name = L"Server";
        message = L"Global chat is currently disabled. You can enable it by typing /global ON.";
    }
    return true;
}

// TODO: Add a joined message if trade chat was off and the user types a message or switches channels
bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    Client& client = Client::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/tr ");

    type = EngineAPI::UI::CHAT_TYPE_TRADE;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
        {
            channel = 1;
            client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, 1);
        }

        name = L"Server";
        message = L"Trade chat is now ON. Joined trade channel " + std::to_wstring(channel) + L".";
        return true;
    }
    else if (arg == L"off")
    {
        client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, 0);
        name = L"Server";
        message = L"Trade chat is now OFF.";
        return true;
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
            client.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
            std::wstring joinMessage = L"Joined trade channel " + std::to_wstring(channel) + L".";
            if (message.empty())
            {
                name = L"Server";
                message = joinMessage;
                return true;
            }
            else
            {
                GameAPI::SendChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_TRADE);
            }
        }
    }

    uint8_t currentChannel = client.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in trade channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            // TODO: Send the message to the server here
        }
    }
    else
    {
        name = L"Server";
        message = L"Trade chat is currently disabled. You can enable it by typing /trade ON.";
    }
    return true;
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
};

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t unk1)
{
    typedef void(__thiscall* HandleSendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    HandleSendChatMessageProto callback = (HandleSendChatMessageProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
        chatWindow.SetChatPrefix({});

        std::wstring realName = name;
        std::wstring realMessage = message;

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

        callback(_this, realName, realMessage, type, targets, unk1);
    }
}
