#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "ItemReplicaInfo.h"
#include "JSON.h"

std::string HandleWriteSendMessage(uint8_t channel, std::wstring message, std::wstring playerName, void* item)
{
    json itemJSON;
    if (item)
        itemJSON = GameAPI::GetItemReplicaInfo(item);

    json request = 
    {
        { "RequestName", "Send" },
        { "Channel", channel },
        { "DirectTo", nullptr },
        { "Message", WideToBase64(message) },
        { "Item", (item) ? itemJSON : nullptr }
    };

    if (!playerName.empty())
    {
        request["DirectTo"] = WideToChar(playerName);
        request["Channel"] = nullptr;
    }
    return request.dump();
}

void HandleReadSendMessage(const json& response)
{
    uint8_t type = CHAT_TYPE_GLOBAL;

    std::wstring playerName;
    if (response.at("From").is_string())
        playerName = CharToWide(response.at("From").get<std::string>());

    // Skip messages from players that are muted
    if ((playerName.empty()) || (spChat->IsPlayerMuted(playerName)))
        return;

    // Error messages
    if (response.at("ErrorMessage").is_string())
    {
        std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        std::wstring message = CharToWide(response.at("ErrorMessage").get<std::string>());
        GameAPI::AddChatMessage(name, message, CHAT_TYPE_SYSTEM);
        return;
    }
    // Direct server announcements
    else if (playerName == L"Admin")
    {
        bool important = false;
        const json& arguments = response.at("Arguments");
        if (arguments.contains("Important"))
            important = arguments.at("Important").get<bool>();

        if ((important) || (spChat->IsServerMessagesEnabled()))
        {
            std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            const json& message = response.at("Message");
            for (const json& line : message)
            {
                std::wstring message = CharToWide(line.get<std::string>());
                GameAPI::AddChatMessage(name, message, CHAT_TYPE_SYSTEM);
            }
        }
        return;
    }
    // Whisper from message
    else if (response.at("DirectUsername").is_string())
    {
        playerName.push_back(0);
        playerName = EngineAPI::UI::Localize("tagGDCLChatDirectFrom", WideToRaw(playerName, false).c_str());
        type = CHAT_TYPE_NORMAL;
    }
    // Whisper to message
    else if (response.at("Channel").is_null())
    {
        playerName.push_back(0);
        playerName = EngineAPI::UI::Localize("tagGDCLChatDirectTo", WideToRaw(playerName, false).c_str());
        type = CHAT_TYPE_NORMAL;
    }
    // Skip other messages if global chat is turned off
    else if (!spChat->IsGlobalChatEnabled())
    {
        return;
    }

    void* item = nullptr;
    if (response.at("Item").is_object())
    {
        ItemReplicaInfo itemInfo = response.at("Item");
        itemInfo._itemID = EngineAPI::CreateObjectID();
        item = GameAPI::CreateItem(itemInfo);
    }

    const json& message = response.at("Message");
    if (message.is_array())
    {
        for (const json& line : message)
        {
            std::wstring message = Base64ToWide(line.get<std::string>());
            GameAPI::AddChatMessage(playerName, message, type, item);
        }
    }
}