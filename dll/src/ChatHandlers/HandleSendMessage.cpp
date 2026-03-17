#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "Item.h"
#include "JSON.h"

std::string HandleWriteSendMessage(uint32_t requestID, uint8_t channel, std::wstring message, std::wstring playerName, void* item)
{
    json itemJSON;
    if (item)
    {
        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(item);
        itemJSON = InfoToItem(itemInfo);
    }

    json request = 
    {
        { "RequestName", "Send" },
        { "RequestId", requestID },
        { "Channel", channel },
        { "DirectTo", (!playerName.empty()) ? playerName : nullptr },
        { "Message", message },
        { "Item", (item) ? itemJSON.dump() : nullptr }
    };
    return request.dump();
}

void HandleReadSendMessage(const json& response, uint8_t channel, std::wstring message, std::wstring playerName, void* item)
{
    // TODO: Handle edge cases like the recipient being offline, etc.
    uint8_t type = ChatAPI::CHAT_TYPE_GLOBAL;
    playerName = response.at("From").get<std::wstring>();
    message = response.at("Message").get<std::wstring>();

    if (ChatAPI::IsPlayerMuted(playerName))
        return;

    if (!response.at("DirectUsername").is_null())
    {
        playerName = L"[From " + playerName + L"]";
        type = ChatAPI::CHAT_TYPE_WHISPER;
    }

    item = nullptr;
    const json& itemJSON = response.at("Item");
    if (!itemJSON.is_null())
    {
        GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo((Item)itemJSON);
        itemInfo._itemID = EngineAPI::CreateObjectID();
        item = GameAPI::CreateItem(itemInfo);
    }

    GameAPI::AddChatMessage(playerName, message, type, item);
}