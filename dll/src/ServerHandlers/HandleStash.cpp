#include <string>
#include "SeasonClient.h"
#include "Item.h"
#include "JSON.h"

std::string HandleWriteGetStashFile(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantStashFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }}
    };
    return request.dump();
}

std::string HandleWriteSaveStashFile(uint32_t participantID, std::string base64Data)
{
    json request = 
    {
        { "RequestName", "SaveParticipantStashFile" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
        }},
        { "File", base64Data }
    };
    return request.dump();
}

std::string HandleWriteStashCapacity()
{
    json request = 
    {
        { "RequestName", "GetParticipantSharedStashCapacity" }
    };
    return request.dump();
}

std::string HandleWriteTransferItems(uint32_t participantID, std::vector<uint32_t> itemIDs)
{
    json request = 
    {
        { "RequestName", "TransferParticipantItems" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "ParticipantItemIds", itemIDs },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

std::string HandleWriteStoreItems(uint32_t participantID, std::vector<Item> items)
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
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }},
        { "Data", itemList }
    };
    return request.dump();
}

std::string HandleWriteTransferQueue(uint32_t participantID)
{
    json request = 
    {
        { "RequestName", "GetParticipantTransferQueue" },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }}
    };
    return request.dump();
}

void HandleReadGetStashFile(json response)
{
    // TODO
}

void HandleReadSaveStashFile(json response)
{
    // TODO
}

void HandleReadStashCapacity(json response)
{
    // TODO
}

void HandleReadTransferItems(json response)
{
    // TODO
}

void HandleReadStoreItems(json response)
{
    // TODO
}

void HandleReadTransferQueue(json response)
{
    // TODO
}