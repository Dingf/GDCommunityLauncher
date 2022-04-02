#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "SharedStash.h"
#include "Log.h"
#include "URI.h"
#include "md5.hpp"

std::filesystem::path GetTransferStashPath(const char* modName, bool hardcore, bool backup)
{
    std::string baseFolder = GameAPI::GetBaseFolder();
    if (baseFolder.empty())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine the base Grim Dawn save location");
        return {};
    }

    std::filesystem::path stashPath = std::filesystem::path(baseFolder) / "save";
    if (modName)
        stashPath /= modName;

    if (backup)
    {
        if (hardcore)
            stashPath /= "transfer.h00";
        else
            stashPath /= "transfer.t00";
    }
    else
    {
        if (hardcore)
            stashPath /= "transfer.gsh";
        else
            stashPath /= "transfer.gst";
    }

    if (!std::filesystem::exists(stashPath))
        return {};

    return stashPath;
}

std::string GenerateStashMD5(const std::filesystem::path& path)
{
    std::stringstream buffer;
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return {};

    buffer << in.rdbuf();

    // Capitalize the MD5 hash to match the server output
    std::string result = websocketpp::md5::md5_hash_hex(buffer.str());
    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        *it = toupper(*it);

    return result;
}

void PostTransferStashUpload()
{
    Client& client = Client::GetInstance();
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
    {
        std::filesystem::path stashPath = GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer), false);
        if (stashPath.empty())
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not determine shared stash path for mod \"%\"", modName);
            return;
        }

        SharedStash stashData;
        if (!stashData.ReadFromFile(stashPath))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not load shared stash data from file");
            return;
        }

        web::json::value stashJSON = stashData.ToJSON();
        web::json::array stashTabs = stashJSON[U("Tabs")].as_array();
        if (stashTabs.size() >= 6)
        {
            web::json::array transferItems = stashTabs[5][U("Items")].as_array();
            if (transferItems.size() > 0)
            {
                std::scoped_lock lock(client.GetTransferMutex());

                uint32_t index = 0;
                web::json::value requestBody = web::json::value::array();
                for (auto it = transferItems.begin(); it != transferItems.end(); ++it)
                {
                    requestBody[index] = *it;
                    requestBody[index].erase(U("Unknown1"));
                    requestBody[index].erase(U("Unknown2"));
                    requestBody[index].erase(U("X"));
                    requestBody[index].erase(U("Y"));
                    index++;
                }

                try
                {
                    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "stash";

                    web::http::client::http_client httpClient((utility::string_t)endpoint);
                    web::http::http_request request(web::http::methods::POST);
                    request.set_body(requestBody);

                    std::string bearerToken = "Bearer " + client.GetAuthToken();
                    request.headers().add(U("Authorization"), bearerToken.c_str());

                    web::http::http_response response = httpClient.request(request).get();
                    if (response.status_code() == web::http::status_codes::OK)
                    {
                        Stash::StashTab* stashTab = stashData.GetStashTab(5);
                        if (stashTab)
                        {
                            // TODO: Possibly check the data returned by the response in case not all of the items could be stored
                            stashTab->GetItemList().clear();
                            stashData.WriteToFile(stashPath);
                            GameAPI::DisplayUINotification("tagGDLeagueStorageSuccess");
                        }
                        else
                        {
                            throw std::runtime_error("Could not retrieve stash tab data from save file");
                        }
                    }
                    else
                    {
                        throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                    }
                }
                catch (const std::exception& ex)
                {
                    GameAPI::DisplayUINotification("tagGDLeagueStorageFailure");
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to upload shared stash items: %", ex.what());
                }
            }
        }
    }
}

void PostTransferStashChecksums(const std::string& prevChecksum)
{
    Client& client = Client::GetInstance();
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
    {
        std::filesystem::path stashPath = GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer), false);
        std::filesystem::path backupPath = GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer), true);

        web::json::value requestBody;
        requestBody[U("lastChecksum")] = JSONString(prevChecksum);
        requestBody[U("sharedStashChecksum")] = JSONString(GenerateStashMD5(stashPath));
        requestBody[U("externalToolUsed")] = std::filesystem::is_regular_file(backupPath);

        pplx::create_task([requestBody]()
        {
            try
            {
                Client& client = Client::GetInstance();
                URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "shared-stash";

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
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update shared stash times: %", ex.what());
            }
        });
    }
}

void HandleSaveTransferStash(void* _this)
{
    typedef void(__thiscall* SaveTransferStashProto)(void*);

    SaveTransferStashProto callback = (SaveTransferStashProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SAVE_TRANSFER_STASH);
    if (callback)
    {
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        std::filesystem::path stashPath = GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer), false);
        std::string prevChecksum = GenerateStashMD5(stashPath);

        callback(_this);

        if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
        {
            pplx::create_task([prevChecksum]()
            {
                PostTransferStashUpload();
                PostTransferStashChecksums(prevChecksum);
            });
        }
    }
}

void PostPullTransferItems(const std::vector<Item*>& items)
{
    Client& client = Client::GetInstance();
    const char* modName = EngineAPI::GetModName();
    PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

    if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
    {
        web::json::value requestBody = web::json::value::array();
        for (uint32_t i = 0; i < items.size(); ++i)
        {
            requestBody[i] = items[i]->_itemID;
        }

        pplx::create_task([requestBody]()
            {
                try
                {
                    Client& client = Client::GetInstance();
                    URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "pull-items";

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
            });
    }
}

void HandleLoadTransferStash(void* _this)
{
    typedef void(__thiscall* LoadPlayerTransferProto)(void*);
    LoadPlayerTransferProto callback = (LoadPlayerTransferProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);

    if (callback)
    {
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

        if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()) && (client.GetTransferMutex().try_lock()))
        {
            try
            {
                URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "transfer-queue";
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

                    std::filesystem::path stashPath = GetTransferStashPath(modName, GameAPI::IsPlayerHardcore(mainPlayer), false);
                    if (stashPath.empty())
                        throw std::runtime_error("Could not determine shared stash path for mod \"" + std::string(modName) + "\"");

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

            client.GetTransferMutex().unlock();
        }

        callback(_this);
    }
}

void HandleSetTransferOpen(void* _this, uint32_t unk1, bool unk2, bool unk3)
{
    Client& client = Client::GetInstance();

    typedef void(__thiscall* SetTransferOpenProto)(void*, uint32_t, bool, bool);

    SetTransferOpenProto callback = (SetTransferOpenProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_ON_CARAVAN_INTERACT);
    if ((callback) && (client.GetTransferMutex().try_lock()))
    {
        client.GetTransferMutex().unlock();
        callback(_this, unk1, unk2, unk3);
    }
}