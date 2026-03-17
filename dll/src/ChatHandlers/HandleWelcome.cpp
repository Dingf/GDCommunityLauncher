#include <string>
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"

std::string HandleWriteWelcome(uint32_t requestID)
{
    json request = 
    {
        { "RequestName", "Welcome" },
        { "RequestId", requestID },
    };
    return request.dump();
}

void HandleReadWelcome(const json& response)
{
    const json& message = response.at("Message");
    if (message.is_array())
    {
        for (const json& line : message)
        {
            GameAPI::AddChatMessage(L"Server", line.get<std::wstring>(), ChatAPI::CHAT_TYPE_NORMAL);
        }
    }
}