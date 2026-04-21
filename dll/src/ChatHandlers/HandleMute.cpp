#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetMutedList()
{
    json request = 
    {
        { "RequestName", "MuteList" }
    };
    return request.dump();
}

std::string HandleWriteMutePlayer(std::wstring playerName)
{
    json request = 
    {
        { "RequestName", "Mute" },
        { "Message", WideToChar(playerName) }
    };
    return request.dump();
}

std::string HandleWriteUnmutePlayer(std::wstring playerName)
{
    json request = 
    {
        { "RequestName", "Unmute" },
        { "Message", WideToChar(playerName) }
    };
    return request.dump();
}

void HandleReadGetMutedList(const json& response)
{
    ChatAPI::ClearMutedList();

    const json& message = response.at("Message");
    for (const json& player : message)
    {
        std::wstring playerName = CharToWide(player.get<std::string>());
        ChatAPI::MutePlayer(playerName);
    }
}

void HandleReadMutePlayer(const json& response)
{
    std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
    std::wstring message;

    const json& arguments = response.at("Arguments");
    if (!arguments.is_null())
    {
        std::wstring playerName = CharToWide(arguments.at("Username").get<std::string>());
        message = EngineAPI::UI::Localize("tagGDCLChatMute01", playerName.c_str());
        ChatAPI::MutePlayer(playerName);
    }
    else
    {
        message = EngineAPI::UI::Localize(response.at("ErrorMessage").get<std::string>().c_str());
    }

    GameAPI::AddChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);

}

void HandleReadUnmutePlayer(const json& response)
{
    std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
    std::wstring message;

    const json& arguments = response.at("Arguments");
    if (!arguments.is_null())
    {
        std::wstring playerName = CharToWide(arguments.at("Username").get<std::string>());
        message = EngineAPI::UI::Localize("tagGDCLChatUnmute01", playerName.c_str());
        ChatAPI::UnmutePlayer(playerName);
    }
    else
    {
        message = EngineAPI::UI::Localize(response.at("ErrorMessage").get<std::string>().c_str());
    }

    GameAPI::AddChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
}