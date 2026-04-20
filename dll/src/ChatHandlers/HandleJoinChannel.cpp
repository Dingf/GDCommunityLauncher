#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"

std::string HandleWriteJoinChannel(uint8_t channel)
{
    json request = 
    {
        { "RequestName", "JoinChannel" },
        { "Channel", channel },
    };
    return request.dump();
}

void HandleReadJoinChannel(const json& response)
{
    const json& arguments = response.at("Arguments");
    if (!arguments.is_null())
    {
        uint32_t channel = arguments.at("Channel").get<uint32_t>();
        std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        std::wstring message = EngineAPI::UI::Localize("tagGDCLChatChannelJoin", channel);

        ChatAPI::SetChatChannel(channel);
        GameAPI::AddChatMessage(name, message, ChatAPI::CHAT_TYPE_GLOBAL);
    }
}