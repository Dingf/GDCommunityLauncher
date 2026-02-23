#include <string>
#include "SeasonClient.h"
#include "Item.h"
#include "JSON.h"

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

void HandleReadGetStashFile(json response, uint32_t participantID)
{
    // TODO
}

void HandleReadSaveStashFile(json response, uint32_t participantID, std::string base64Data)
{
    // TODO
}

void HandleReadStashCapacity(json response)
{
    // TODO
}

void HandleReadTransferItems(json response, uint32_t participantID, std::vector<uint32_t> itemIDs)
{
    // TODO
}

void HandleReadStoreItems(json response, uint32_t participantID, std::vector<Item> items)
{
    // TODO
}

void HandleReadTransferQueue(json response, uint32_t participantID)
{
    // TODO
}