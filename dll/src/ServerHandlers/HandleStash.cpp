#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "DllClient.h"
#include "ServerCache.h"
#include "ServerHandler.h"
#include "StringConvert.h"
#include "Item.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetStashFile(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantStashFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveStashFile(uint32_t requestID, uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantStashFile" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteStashCapacity(uint32_t requestID)
{
    json request = 
    {
        { "RequestName", "GetParticipantSharedStashCapacity" },
        { "RequestId", requestID },
    };
    return request.dump();
}

std::string HandleWriteTransferItems(uint32_t requestID, uint32_t participantID, std::vector<uint32_t> itemIDs)
{
    json request = 
    {
        { "RequestName", "TransferParticipantItems" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "ParticipantItemIds", itemIDs },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

std::string HandleWriteStoreItems(uint32_t requestID, uint32_t participantID, std::vector<Item> items)
{
    std::vector<json> itemList;
    for (size_t i = 0; i < items.size(); ++i)
    {
        json item = items[i];
        item.erase("Unknown1");
        item.erase("Unknown2");
        itemList.emplace_back(item);
    }

    json request = 
    {
        { "RequestName", "StoreParticipantStashItems" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }},
        { "Data", itemList }
    };
    return request.dump();
}

std::string HandleWriteTransferQueue(uint32_t requestID, uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTransferQueue" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

void HandleReadGetStashFile(const json& response, uint32_t participantID)
{
    try
    {
        std::string status = response.at("Status").get<std::string>();
        if (status != "Ok")
            throw std::runtime_error(response.at("ErrorMessage"));

        const json& file = response.at("File");
        if (file.is_null())
            return;

        std::string base64Data = file.get<std::string>();
        std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

        bool hardcore = spCache->IsParticipantHardcore(participantID);
        spCache->SetStashData(hardcore, &binaryData[0], binaryData.size());
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared tags file: %", ex.what());
    }
}

void HandleReadSaveStashFile(const json& response, uint32_t participantID, std::string base64Data)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save shared stash file: %", response.at("ErrorMessage"));
    }
}

void HandleReadStashCapacity(const json& response)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        int32_t capacity = response.at("Data").get<int32_t>();
        spCache->SetStashCapacity(capacity);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash capacity: %", response.at("ErrorMessage"));
    }
}

void HandleReadTransferItems(const json& response, uint32_t participantID, std::vector<uint32_t> itemIDs)
{
    std::string status = response.at("Status").get<std::string>();
    if (status != "Ok")
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to remove items from transfer queue: %", response.at("ErrorMessage"));
    }
}

void HandleReadStoreItems(const json& response, uint32_t participantID, std::vector<Item> items)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
        if (transferTabs.size() >= 6)
        {
            void* uploadTab = transferTabs[5];
            GameAPI::RemoveAllItemsFromTab(uploadTab);
            GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
        }
    }
    else
    {
        // TODO: Handle the case where the user exceeds the number of items in their cloud stash
    }

    GameAPI::SetTransferLocked(false);
}

void HandleReadTransferQueue(const json& response, uint32_t participantID)
{
    const std::string status = response.at("Status").get<std::string>();
    if (status == "Ok")
    {
        const json& itemsArray = response.at("Data");
        if ((GameAPI::GetTransferTabs().size() >= 6) && (itemsArray.size() > 0))
        {
            std::vector<uint32_t> pulledItemIDs;
            uint32_t playerID = EngineAPI::GetObjectID(GameAPI::GetMainPlayer());

            for (const auto& itemJSON : itemsArray)
            {
                std::shared_ptr<Item> itemData = std::make_shared<Item>(itemJSON);
                GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo(*itemData);
                if (void* item = GameAPI::CreateItem(itemInfo))
                {
                    GameAPI::SetItemVisiblePlayer(item, playerID);
                    if (GameAPI::AddItemToTransfer(EngineAPI::GetObjectID(item), 4, true))
                        pulledItemIDs.push_back(itemData->_itemID);

                    EngineAPI::DestroyObjectEx(item);
                }
                else
                {
                    throw std::runtime_error("Failed to recreate stash item from item data");
                }
            }

            GameAPI::SaveTransferStash();
            spServer->Send("TransferParticipantItems", participantID, pulledItemIDs);
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to retrieve items from transfer queue: %", response.at("ErrorMessage"));
    }
}