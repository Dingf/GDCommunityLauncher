#include <string>
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"

std::string HandleWriteOnline(uint32_t requestID)
{
    json request = 
    {
        { "RequestName", "Online" },
        { "RequestId", requestID },
    };
    return request.dump();
}

void HandleReadOnline(const json& response)
{
    const json& message = response.at("Message");
    if (!message.is_null())
    {
        uint32_t count = message.get<uint32_t>();
        std::wstring message = L"There are " + std::to_wstring(count) + L" users currently online.";
        GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);
    }
}