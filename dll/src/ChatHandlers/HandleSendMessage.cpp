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
        { "Message", WideToRaw(message) },
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
    uint8_t type = ChatAPI::CHAT_TYPE_GLOBAL;

    std::wstring playerName;
    if (response.at("From").is_string())
        playerName = CharToWide(response.at("From").get<std::string>());

    if ((playerName.empty()) || (ChatAPI::IsPlayerMuted(playerName)))
        return;

    if (response.at("ErrorMessage").is_string())
    {
        std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        std::wstring message = CharToWide(response.at("ErrorMessage").get<std::string>());
        GameAPI::AddChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
        return;
    }
    else if (response.at("DirectUsername").is_string())
    {
        playerName = EngineAPI::UI::Localize("tagGDCLChatDirectFrom", playerName);
        type = ChatAPI::CHAT_TYPE_NORMAL;
    }
    else if (response.at("Channel").is_null())
    {
        playerName = EngineAPI::UI::Localize("tagGDCLChatDirectTo", playerName);
        type = ChatAPI::CHAT_TYPE_NORMAL;
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
            std::wstring message = RawToWide(line.get<std::string>());
            GameAPI::AddChatMessage(playerName, message, type, item);
        }
    }
}