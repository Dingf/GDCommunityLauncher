#include <unordered_map>
#include <regex>
#include <cwctype>
#include "ClientHandlers.h"

const uint32_t CHAT_CHANNEL_MAX = 15;

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    GameAPI::SendChatMessage(L"/g, /global [ON/OFF]", L"Enables or disables the global chat channel, or sends a message to the global chat channel.", 0);
    GameAPI::SendChatMessage(L"/tr, /trade [ON/OFF]", L"Enables or disables the trade chat channel, or sends a message to the trade chat channel.", 0);
    GameAPI::SendChatMessage(L"/h, /help", L"Displays this help message.", 0);
    return false;
}

//TODO: Send the global/trade chat messages to the server and also add listeners on the client for incoming messages
bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        name = L"Server";
        message = L"Global chat is now ON. Joining global channel " + std::to_wstring(channel) + L".";
        type = 2;
    }
    else if (arg == L"off")
    {
        name = L"Server";
        message = L"Global chat is now OFF.";
        type = 2;
    }
    else if (channel != 0)
    {
        if (channel > CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(CHAT_CHANNEL_MAX) + L".";
            type = 2;
        }
        else
        {
            std::wstring joinMessage = L"Joining global channel " + std::to_wstring(channel) + L".";
            if (message.empty())
            {
                name = L"Server";
                message = joinMessage;
                type = 2;
                return true;
            }
            else
            {
                GameAPI::SendChatMessage(L"Server", joinMessage, 2);
            }
        }
    }

    Client& client = Client::GetInstance();
    name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
    type = 2;
    return true;
}

// TODO: Add a joined message if trade chat was off and the user types a message or switches channels
bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        name = L"Server";
        message = L"Trade chat is now ON. Joining trade channel " + std::to_wstring(channel) + L".";
        type = 1;
        return true;
    }
    else if (arg == L"off")
    {
        name = L"Server";
        message = L"Trade chat is now OFF.";
        type = 1;
        return true;
    }
    else if (channel != 0)
    {
        if (channel > CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(CHAT_CHANNEL_MAX) + L".";
            type = 1;
            return true;
        }
        else
        {
            std::wstring joinMessage = L"Joining trade channel " + std::to_wstring(channel) + L".";
            if (message.empty())
            {
                name = L"Server";
                message = joinMessage;
                type = 1;
                return true;
            }
            else
            {
                GameAPI::SendChatMessage(L"Server", joinMessage, 1);
            }
        }
    }

    Client& client = Client::GetInstance();
    name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
    type = 1;
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
