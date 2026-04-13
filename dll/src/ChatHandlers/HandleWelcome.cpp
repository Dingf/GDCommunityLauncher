#include <string>
#include "GameAPI.h"
#include "ChatAPI.h"
#include "StringConvert.h"
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
            std::wstring message = CharToWide(line.get<std::string>());
            GameAPI::AddChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);
        }
    }
}