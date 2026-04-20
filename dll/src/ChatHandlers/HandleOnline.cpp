#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteOnline()
{
    json request = 
    {
        { "RequestName", "Online" }
    };
    return request.dump();
}

void HandleReadOnline(const json& response)
{
    const json& message = response.at("Message");
    if (message.is_array())
    {
        try
        {
            uint32_t count = std::stoi(message[0].get<std::string>());
            std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatOnline", count);
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to parse number of online players");
        }
    }
}