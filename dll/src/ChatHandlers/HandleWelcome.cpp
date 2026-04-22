#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "JSON.h"

std::string HandleWriteWelcome()
{
    json request = 
    {
        { "RequestName", "Welcome" }
    };
    return request.dump();
}

void HandleReadWelcome(const json& response)
{
    const json& message = response.at("Message");
    if (message.is_array())
    {
        std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        for (const json& line : message)
        {
            std::wstring message = CharToWide(line.get<std::string>());
            GameAPI::AddChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
        }

        if (!ChatAPI::IsServerMessagesEnabled())
            GameAPI::AddChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatAnnouncementsDisabled"), ChatAPI::CHAT_TYPE_SYSTEM);

        if (ChatAPI::GetChatChannel() == 0)
            GameAPI::AddChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatGlobalDisabled"), ChatAPI::CHAT_TYPE_GLOBAL);
    }
}