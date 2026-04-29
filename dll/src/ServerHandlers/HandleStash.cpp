#include <string>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "DllClient.h"
#include "ServerCache.h"
#include "ServerHandler.h"
#include "ItemReplicaInfo.h"
#include "HTTP.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetStashFile(uint32_t requestID, uint32_t& participantID)
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

std::string HandleWriteSaveStashFile(uint32_t requestID, uint32_t& participantID, std::string& base64Data)
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
    base64Data.clear();
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

std::string HandleWriteTransferItems(uint32_t requestID, uint32_t& participantID, std::vector<uint32_t>& itemIDs)
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
    itemIDs.clear();
    return request.dump();
}

std::string HandleWriteStoreItems(uint32_t requestID, uint32_t& participantID, std::vector<json>& items)
{
    json request = 
    {
        { "RequestName", "StoreParticipantStashItems" },
        { "RequestId", requestID },
        { "Arguments", {
            { "SeasonParticipantId", participantID },
            { "Branch", spClient->GetBranchName() }
        }},
        { "Data", items }
    };
    items.clear();
    return request.dump();
}

std::string HandleWriteTransferQueue(uint32_t requestID, uint32_t& participantID, uint32_t& caravanID)
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
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            const json& file = response.at("File");
            std::string base64Data = file.get<std::string>();
            std::vector<uint8_t> binaryData = Base64ToBinary(base64Data);

            bool hardcore = spCache->IsParticipantHardcore(participantID);
            spCache->SetStashData(hardcore, &binaryData[0], binaryData.size());
        }
        else if (status != HTTP_STATUS_NO_CONTENT)
        {
            throw std::runtime_error(response.at("ErrorMessage"));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared tags file: %", ex.what());
    }
}

void HandleReadSaveStashFile(const json& response, uint32_t participantID, std::string base64Data)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status != HTTP_STATUS_OK)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to save shared stash file: %", response.at("ErrorMessage"));
    }
}

void HandleReadStashCapacity(const json& response)
{
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status == HTTP_STATUS_OK)
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
    HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
    if (status != HTTP_STATUS_OK)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to remove items from transfer queue: %", response.at("ErrorMessage"));
    }
}

void HandleReadStoreItems(const json& response, uint32_t participantID, std::vector<json> items)
{
    try
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
            if (transferTabs.size() >= 6)
            {
                void* uploadTab = transferTabs[5];
                GameAPI::RemoveAllItemsFromTab(uploadTab);
                GameAPI::SaveTransferStash();
                GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
            }
        }
        else if (status == HTTP_STATUS_BAD_REQUEST)
        {
            GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
        }
        else
        {
            GameAPI::DisplayUINotification("tagGDLeagueStorageFailure");
            throw std::runtime_error(response.at("ErrorMessage"));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to upload items to cloud stash: %", ex.what());
    }

    GameAPI::SetTransferLocked(false);
}

void HandleReadTransferQueue(const json& response, uint32_t participantID, uint32_t caravanID)
{
    try
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            const json& itemsArray = response.at("Data");
            if ((GameAPI::GetTransferTabs().size() >= 6) && (itemsArray.size() > 0))
            {
                std::vector<uint32_t> pulledItemIDs;
                uint32_t playerID = EngineAPI::GetObjectID(GameAPI::GetMainPlayer());

                for (const auto& itemJSON : itemsArray)
                {
                    ItemReplicaInfo itemInfo = itemJSON;
                    if (void* item = GameAPI::CreateItem(itemInfo))
                    {
                        GameAPI::SetItemVisiblePlayer(item, playerID);
                        if (GameAPI::AddItemToTransfer(EngineAPI::GetObjectID(item), 4, true))
                            pulledItemIDs.push_back(itemInfo._participantItemID);

                        EngineAPI::DestroyObjectEx(item);
                    }
                    else
                    {
                        throw std::runtime_error("Failed to recreate stash item from item data");
                    }
                }

                spServer->Send("TransferParticipantItems", participantID, pulledItemIDs);
                GameAPI::SaveTransferStash();
            }

            // Calling DisplayCaravanWindow() here can lead to a race condition since this is in a separate thread
            // So just save the caravan ID so that we can display the window on the next frame in the main thread
            GameAPI::SetLastCaravanID(caravanID);
        }
        else
        {
            throw std::runtime_error(response.at("ErrorMessage"));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to retrieve items from transfer queue: %", ex.what());
    }
}