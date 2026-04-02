#include <string>
#include "ChatAPI.h"
#include "GameAPI.h"
#include "JSON.h"

std::string HandleWriteJoinChannel(uint32_t requestID, uint8_t& channel)
{
    json request = 
    {
        { "RequestName", "JoinChannel" },
        { "RequestId", requestID },
        { "Channel", channel },
    };
    return request.dump();
}

void HandleReadJoinChannel(const json& response, uint8_t channel)
{
    const json& message = response.at("Message");
    if (!message.is_null())
    {
        std::wstring joinMessage = L"Joined global channel " + std::to_wstring(channel) + L".";

        ChatAPI::SetChatChannel(channel);
        GameAPI::AddChatMessage(L"Server", joinMessage, ChatAPI::CHAT_TYPE_GLOBAL);
    }
}