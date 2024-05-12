#ifndef INC_GDCL_DLL_SERVER_SYNC_H
#define INC_GDCL_DLL_SERVER_SYNC_H

#include <atomic>
#include <string>
#include <map>
#include <unordered_set>
#include <filesystem>
#include <ppl.h>
#include "FileData.h"
#include "SharedStash.h"
#include "GameAPI/Difficulty.h"
#include "GameAPI/Item.h"

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
        struct CharacterBuffer : public FileWriter
        {
            CharacterBuffer(uint8_t* buffer, size_t size) : FileWriter(buffer, size) {}

            web::json::value _tagData;
        };

        struct PlayerDataBuffer : public FileWriter
        {
            PlayerDataBuffer(uint8_t* buffer, size_t size, GameAPI::Difficulty difficulty, const std::wstring& playerName) : FileWriter(buffer, size)
            {
                _difficulty = difficulty;
                _playerName = playerName;
            }

            GameAPI::Difficulty _difficulty;
            std::wstring        _playerName;
        };

        struct StashBuffer : public FileWriter
        {
            StashBuffer(uint8_t* buffer, size_t size, bool hardcore) : FileWriter(buffer, size), _hardcore(hardcore) {}

            bool _hardcore;
        };

        ServerSync();
        ServerSync(ServerSync&) = delete;
        void operator=(const ServerSync&) = delete;

        static ServerSync& GetInstance();

        uint32_t GetParticipantID(bool hardcore);
        uint32_t GetCharacterID(const std::wstring& playerName);

        FileMetadata GetServerCharacterMetadata(const std::wstring& playerName, uint32_t participantID);
        FileMetadata GetServerStashMetadata(uint32_t participantID);
        int32_t GetServerStashCapacity(uint32_t participantID);

        void IncrementStashLock() { _stashLock.fetch_add(1); }
        void DecrementStashLock() { _stashLock.fetch_sub(1); }

        void SetStashSynced(StashSyncFlag flag) { _stashSynced.fetch_or(flag); }
        void ResetStashSynced() { _stashSynced.store(0); }

        void SyncCharacterData(const std::filesystem::path& filePath, void** data, size_t* size);
        void SyncStashData();

        void CleanupSaveFolder(const std::unordered_set<std::wstring>& characterList);

        void DownloadCharacterBuffer(std::wstring playerName, uint32_t participantID, void** data, size_t* size);
        void DownloadCharacterFile(std::wstring playerName, uint32_t participantID, bool overwrite = false);
        void DownloadCharacterQuestData(const std::wstring& playerName, uint32_t participantID, bool overwrite = false);
        void DownloadCharacterConversationsData(const std::wstring& playerName, uint32_t participantID, bool overwrite = false);
        void DownloadCharacterFOWData(const std::wstring& playerName, uint32_t participantID, bool overwrite = false);
        void DownloadCharacterList(uint32_t participantID, std::unordered_set<std::wstring>& characterList);
        void DownloadFormulasFile(uint32_t participantID, bool hardcore);
        void DownloadTransmutesFile(uint32_t participantID, bool hardcore);
        void DownloadStashData(uint32_t participantID);
        void DownloadTransferItems(uint32_t participantID);

        static void OnInitializeEvent();
        static void OnShutdownEvent();
        static void OnDirectReadEvent(std::string filename, void** data, size_t* size);
        static void OnDirectWriteEvent(std::string filename, void* data, size_t size);
        static void OnAddSaveJobEvent(std::string filename, void* data, size_t size);
        static void OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded);
        static void OnWorldPostLoadEvent(std::string mapName, bool unk1, bool modded);
        static void OnWorldPreUnloadEvent();
        static void OnSetMainPlayerEvent(void* player);
        static void OnTransferPostLoadEvent();
        static void OnTransferPreSaveEvent();
        static void OnTransferPostSaveEvent();
        static void OnCharacterPreSaveEvent(void* player);
        static void OnCharacterPostSaveEvent(void* player);
        static void OnDeleteFileEvent(const char* filename);
        static void OnDelayedCharacterUpload();

        static void OnAddParticipant(const signalr::value& value);
        static void OnGetCharacterInfo(const signalr::value& value, const std::vector<void*> args);
        static void OnGetCharacterList( const signalr::value& value, const std::vector<void*> args);
        //static void OnGetCharacterFile(const signalr::value& value, const std::vector<void*> args);
        static void OnGetStashInfo(const signalr::value& value, const std::vector<void*> args);
        static void OnConnectionStatus(const signalr::value& value, const std::vector<void*> args);
        //static void OnGetStashFile(const signalr::value& value, const std::vector<void*> args);
        //static void OnUploadCloudStash(const signalr::value& value, const std::vector<void*> args);
        //static void OnUploadCharacter(const signalr::value& value, const std::vector<void*> args);
        //static void OnUploadSharedStash(const signalr::value& value, const std::vector<void*> args);
        //static void OnGetTransferQueue(const signalr::value& value, const std::vector<void*> args);
        //static void OnTransferItems(const signalr::value& value, const std::vector<void*> args);

        void RegisterSeasonParticipant(bool hardcore, bool async = true);
        void UploadCloudStash();
        void UploadNewCharacterBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadStashBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadFormulasBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadTransmutesBuffer(const std::string& filename, void* buffer, size_t size);
        void CacheCharacterBuffer(const std::string& filename, void* buffer, size_t size);
        void CacheQuestBuffer(const std::string& filename, void* buffer, size_t size);
        void CacheConversationsBuffer(const std::string& filename, void* buffer, size_t size);
        void CacheFOWBuffer(const std::string& filename, void* buffer, size_t size);
        void UploadCachedCharacterBuffer();
        void UploadCachedQuestBuffer();
        void UploadCachedConversationsBuffer();
        void UploadCachedFOWBuffer();
        void UploadCachedStashBuffer();
        void UploadCachedBuffers();
        void SaveTagsFile();
        void LoadQuestStatesForPlayer(void* player);
        void LoadTagsFileForPlayer(void* player);
        void PullTransferItems(const std::vector<std::shared_ptr<Item>>& items);

        static void WaitBackgroundComplete();

        std::atomic_int32_t _stashSynced;   // Bit flag used to indicate whether the stash was successfully synced with the server data
        std::atomic_int32_t _stashLock;     // Lock used to prevent the user from opening the stash during stash operations
        std::shared_ptr<CharacterBuffer>  _cachedCharacterBuffer;
        std::shared_ptr<PlayerDataBuffer> _cachedQuestBuffer;
        std::shared_ptr<PlayerDataBuffer> _cachedConversationsBuffer;
        std::shared_ptr<PlayerDataBuffer> _cachedFOWBuffer;
        std::shared_ptr<StashBuffer>      _cachedStashBuffer;
        std::map<std::wstring, uint32_t> _characterIDCache;
        std::map<bool, uint32_t> _participantIDCache;
        concurrency::task_group _backgroundTasks;
};


#endif//INC_GDCL_DLL_SERVER_SYNC_H