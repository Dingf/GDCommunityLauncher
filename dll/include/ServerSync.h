#ifndef INC_GDCL_DLL_SERVER_SYNC_H
#define INC_GDCL_DLL_SERVER_SYNC_H

#include <atomic>
#include <string>
#include <map>
#include <filesystem>
#include <ppl.h>
#include "FileData.h"
#include "SharedStash.h"
#include "Item.h"

enum StashSyncFlag
{
    STASH_SYNC_DOWNLOAD = 0x01,
    STASH_SYNC_TRANSFER = 0x02,
};

class ServerSync
{
    public:
        static void Initialize() { GetInstance(); }

        static bool IsStashSynced() { return (GetInstance()._stashSynced.load() == 0x03); }
        static bool IsStashLocked() { return (GetInstance()._stashLock.load() > 0); }

        template<typename TaskFunc>
        static void ScheduleTask(TaskFunc task)
        {
            ServerSync::GetInstance()._backgroundTasks.run(task);
        }

    private:
        struct CharacterIDRef
        {
            bool operator<(const CharacterIDRef& rhs) const
            {
                if (_participantID == rhs._participantID)
                    return (_name < rhs._name);
                else
                    return (_participantID < rhs._participantID);
            }

            std::wstring _name;
            uint32_t _participantID;
        };

        ServerSync();
        ServerSync(ServerSync&) = delete;
        void operator=(const ServerSync&) = delete;

        static ServerSync& GetInstance();

        uint32_t GetParticipantID(bool hardcore);
        uint32_t GetCharacterID(const std::wstring& playerName, uint32_t participantID = 0);

        FileMetadata GetServerCharacterMetadata(const std::wstring& playerName, uint32_t participantID);
        FileMetadata GetServerStashMetadata(uint32_t participantID);

        void IncrementStashLock() { _stashLock.fetch_add(1); }
        void DecrementStashLock() { _stashLock.fetch_sub(1); }

        void SetStashSynced(StashSyncFlag flag) { _stashSynced.fetch_or(flag); }
        void ResetStashSynced() { _stashSynced.store(0); }

        void SyncCharacterData(const std::filesystem::path& filePath, void** data, size_t* size);
        void SyncStashData();

        void DownloadCharacterBuffer(const std::wstring& playerName, uint32_t participantID, void** data, size_t* size);
        void DownloadCharacterFile(const std::wstring& playerName, uint32_t participantID);
        void DownloadCharacterQuestFile(const std::wstring& playerName, uint32_t participantID);
        void DownloadCharacterList(uint32_t participantID);
        void DownloadStashData(uint32_t participantID);
        void DownloadTransferItems(uint32_t participantID);

        static void OnShutdownEvent();
        static void OnDirectReadEvent(std::string filename, void** data, size_t* size, bool* override);
        static void OnDirectWriteEvent(std::string filename, void* data, size_t size);
        static void OnAddSaveJob(std::string filename, void* data, size_t size);
        static void OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded);
        static void OnWorldPostLoadEvent(std::string mapName, bool unk1, bool modded);
        static void OnWorldPreUnloadEvent();
        static void OnTransferPostLoadEvent();
        static void OnTransferPreSaveEvent();
        static void OnTransferPostSaveEvent();
        static void OnCharacterPreSaveEvent(void* player);
        static void OnCharacterPostSaveEvent(void* player);
        static void OnDelayedCharacterUpload();

        void RegisterSeasonParticipant(bool hardcore);
        void UploadCharacterQuestData(uint32_t participantID, const std::wstring& characterName);
        void UploadCloudStash();
        void UploadNewCharacterBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadStashBuffer(const std::string& filename, void* buffer, size_t size);
        void CacheCharacterBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadCachedStashBuffer();
        void UploadCachedCharacterBuffer();
        void PullTransferItems(const std::vector<std::shared_ptr<Item>>& items);

        static void WaitBackgroundComplete();

        void*        _newPlayer;
        std::atomic_int32_t _stashSynced;   // Bit flag used to indicate whether the stash was successfully synced with the server data
        std::atomic_int32_t _stashLock;     // Lock used to prevent the user from opening the stash during stash operations
        std::map<CharacterIDRef, uint32_t> _characterIDCache;
        std::unique_ptr<FileWriter> _cachedStashBuffer;
        std::unique_ptr<FileWriter> _cachedCharacterBuffer;
        std::unique_ptr<web::json::value> _cachedQuestData;
        concurrency::task_group _backgroundTasks;
};


#endif//INC_GDCL_DLL_SERVER_SYNC_H