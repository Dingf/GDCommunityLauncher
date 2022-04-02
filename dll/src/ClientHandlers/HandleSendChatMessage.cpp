#include <unordered_map>
#include <regex>
#include <cwctype>
#include "ClientHandlers.h"

//TODO: Send the global/trade chat messages to the server and also add listeners on the client for incoming messages
bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint8_t& channel)
{
    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        name = L"Server";
        message = L"Global chat is now ON.";
        channel = 2;
    }
    else if (arg == L"off")
    {
        name = L"Server";
        message = L"Global chat is now OFF.";
        channel = 2;
    }
    else
    {
        Client& client = Client::GetInstance();
        name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
        channel = 2;
    }

    return true;
}

bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint8_t& channel)
{
    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        name = L"Server";
        message = L"Trade chat is now ON.";
        channel = 1;
    }
    else if (arg == L"off")
    {
        name = L"Server";
        message = L"Trade chat is now OFF.";
        channel = 1;
    }
    else
    {
        Client& client = Client::GetInstance();
        name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
        channel = 1;
    }
    return true;
}

typedef bool(*ChatCommandHandler)(std::wstring&, std::wstring&, uint8_t&);
const std::unordered_map<std::wstring, ChatCommandHandler> chatCommandLookup =
{
    { L"global", HandleChatGlobalCommand },
    { L"g",      HandleChatGlobalCommand },
    { L"trade",  HandleChatTradeCommand },
    { L"tr",     HandleChatTradeCommand },
};

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t channel, std::vector<uint32_t> targets, uint32_t unk1)
{
    typedef void(__thiscall* HandleSendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    HandleSendChatMessageProto callback = (HandleSendChatMessageProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        std::wstring realName = name;
        std::wstring realMessage = message;

        std::wsmatch match;
        std::wregex commandRegex(L"^\\/(\\w+)\\s*(.*)$");
        if (std::regex_match(message, match, commandRegex))
        {
            std::wstring command = match.str(1);
            std::transform(command.begin(), command.end(), command.begin(), std::towlower);
            realMessage = match.str(2);

            if (chatCommandLookup.count(command) > 0)
            {
                ChatCommandHandler handler = chatCommandLookup.at(command);
                if (!handler(realName, realMessage, channel))
                    return;
            }
        }

        callback(_this, realName, realMessage, channel, targets, unk1);
    }
}
