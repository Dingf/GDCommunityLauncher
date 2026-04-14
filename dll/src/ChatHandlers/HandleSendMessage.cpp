#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "ItemReplicaInfo.h"
#include "StringConvert.h"
#include "JSON.h"

std::string HandleWriteSendMessage(uint32_t requestID, uint8_t& channel, std::wstring& message, std::wstring& playerName, void*& item)
{
    json itemJSON;
    if (item)
        itemJSON = GameAPI::GetItemReplicaInfo(item);

    json request = 
    {
        { "RequestName", "Send" },
        { "RequestId", requestID },
        { "Channel", channel },
        { "DirectTo", nullptr },
        { "Message", WideToRaw(message) },
        { "Item", (item) ? itemJSON : nullptr }
    };

    if (!playerName.empty())
        request["DirectTo"] = WideToChar(playerName);

    return request.dump();
}

void HandleReadSendMessage(const json& response, uint8_t channel, std::wstring message, std::wstring playerName, void* item)
{
    // TODO: Handle edge cases like the recipient being offline, etc.
    uint8_t type = ChatAPI::CHAT_TYPE_GLOBAL;
    playerName = CharToWide(response.at("From").get<std::string>());

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
        ItemReplicaInfo itemInfo = itemJSON;
        itemInfo._itemID = EngineAPI::CreateObjectID();
        item = GameAPI::CreateItem(itemInfo);
    }

    const json& messageJSON = response.at("Message");
    if (messageJSON.is_array())
    {
        for (const json& line : messageJSON)
        {
            message = RawToWide(line.get<std::string>());
            GameAPI::AddChatMessage(playerName, message, type, item);
        }
    }
}