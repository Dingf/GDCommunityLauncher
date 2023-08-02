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
            if (client.IsInProductionBranch())
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

void HandleLoadTransferStash(void* _this)
{
    typedef void (__thiscall* LoadPlayerTransferProto)(void*);
    LoadPlayerTransferProto callback = (LoadPlayerTransferProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_LOAD_TRANSFER_STASH);

    if (callback)
    {
        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            if (GetTransferMutex().try_lock())
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

                    if (client.IsInProductionBranch())
                        ServerSync::SyncStashData(stashPath, hardcore);
                    else
                        ServerSync::DownloadTransferItems(modName, hardcore, participantID);
                }
                catch (const std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve transfer queue items: %", ex.what());
                }
                callback(_this);
                GetTransferMutex().unlock();
            }
        }
        else
        {
            callback(_this);
        }
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