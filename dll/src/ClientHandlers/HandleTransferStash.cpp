#include <filesystem>
#include <thread>
#include <mutex>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ServerSync.h"
#include "SharedStash.h"
#include "URI.h"

std::mutex& GetTransferMutex()
{
    static std::mutex transferMutex;
    return transferMutex;
}

void HandleSaveTransferStash(void* _this)
{
    typedef void (__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsParticipatingInSeason()) && (GetTransferMutex().try_lock()))
        {
            std::string modName = EngineAPI::GetModName();
            void* mainPlayer = GameAPI::GetMainPlayer();
            ServerSync::SnapshotStashMetadata(modName, GameAPI::IsPlayerHardcore(mainPlayer));

            callback(_this);

            // this needs to be a thread or we have to untangle all the continuations inside
            std::thread stashUploader([]()
            {
                Client& client = Client::GetInstance();
                ServerSync::UploadStashData();
                GetTransferMutex().unlock();
            });
            stashUploader.detach();
        }
        else
        {
            callback(_this);
        }
    }
}

void PostPullTransferItems(const std::vector<Item*>& items)
{
    Client& client = Client::GetInstance();
    if (client.IsParticipatingInSeason())
    {
        web::json::value requestBody = web::json::value::array();
        for (uint32_t i = 0; i < items.size(); ++i)
        {
            requestBody[i] = items[i]->_itemID;
        }

        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "pull-items";
            endpoint.AddParam("branch", client.GetBranch());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);
            request.set_body(requestBody);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() != web::http::status_codes::OK)
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to pull items from transfer queue: %", ex.what());
        }
    }
}

void HandleLoadTransferStash(void* _this)
{
    typedef void (__thiscall* LoadPlayerTransferProto)(void*);
    LoadPlayerTransferProto callback = (LoadPlayerTransferProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);

    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsParticipatingInSeason()) && (GetTransferMutex().try_lock()))
        {
            try
            {
                std::string modName = EngineAPI::GetModName();
                void* mainPlayer = GameAPI::GetMainPlayer();
                bool hardcore = GameAPI::IsPlayerHardcore(mainPlayer);
                uint32_t participantID = client.GetParticipantID();

                std::filesystem::path stashPath = GameAPI::GetTransferStashPath(modName, hardcore);
                if (stashPath.empty())
                    throw std::runtime_error("Could not determine shared stash path for mod \"" + std::string(modName) + "\"");

                ServerSync::SyncStashData(stashPath, hardcore);

                URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(participantID) / "transfer-queue";
                endpoint.AddParam("branch", client.GetBranch());

                web::http::client::http_client httpClient((utility::string_t)endpoint);
                web::http::http_request request(web::http::methods::GET);

                std::string bearerToken = "Bearer " + client.GetAuthToken();
                request.headers().add(U("Authorization"), bearerToken.c_str());

                web::http::http_response response = httpClient.request(request).get();
                if (response.status_code() == web::http::status_codes::OK)
                {
                    std::vector<Item> itemList;
                    web::json::array itemsArray = response.extract_json().get().as_array();
                    for (auto it = itemsArray.begin(); it != itemsArray.end(); ++it)
                    {
                        itemList.emplace_back(*it);
                    }

                    SharedStash stashData;
                    if (!stashData.ReadFromFile(stashPath))
                        throw std::runtime_error("Could not load shared stash data from file");

                    if (stashData.GetTabCount() >= 6)
                    {
                        Stash::StashTab* transferTab = stashData.GetStashTab(4);
                        if (transferTab)
                        {
                            std::vector<Item*> pullItemList = transferTab->AddItemList(itemList);
                            stashData.WriteToFile(stashPath);
                            PostPullTransferItems(pullItemList);
                            ServerSync::RefreshStashMetadata(modName, hardcore, participantID);
                        }
                        else
                        {
                            throw std::runtime_error("Could not load transfer tab from shared stash file");
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                }
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve transfer queue items: %", ex.what());
            }
            GetTransferMutex().unlock();
        }

        callback(_this);
    }
}

void HandleSetTransferOpen(void* _this, uint32_t unk1, bool unk2, bool unk3)
{
    typedef void (__thiscall* SetTransferOpenProto)(void*, uint32_t, bool, bool);

    Client& client = Client::GetInstance();
    if((client.IsInActiveSeason()) && (!client.IsParticipatingInSeason()))
        return;

    SetTransferOpenProto callback = (SetTransferOpenProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT);
    if ((callback) && (GetTransferMutex().try_lock()))
    {
        GetTransferMutex().unlock();
        callback(_this, unk1, unk2, unk3);
    }
}