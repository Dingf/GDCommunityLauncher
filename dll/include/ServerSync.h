#ifndef INC_GDCL_DLL_SERVER_SYNC_H
#define INC_GDCL_DLL_SERVER_SYNC_H

#include <string>
#include <map>
#include <filesystem>
#include <ppl.h>
#include "FileData.h"
#include "Item.h"

class ServerSync
{
    public:
        static void Initialize() { GetInstance(); }

        static bool IsClientTrusted();

        static void SnapshotCharacterMetadata(const std::wstring& playerName);
        static void SnapshotStashMetadata(const std::string& modName, bool hardcore);

        static void SyncCharacterData(const std::filesystem::path& filePath);
        static void SyncStashData(const std::filesystem::path& filePath, bool hardcore);

        static void UploadCharacterData(bool buffered);
        static void UploadStashData();

        static bool DownloadCharacterData(const std::wstring& playerName, uint32_t participantID);
        static bool DownloadStashData(const std::string& modName, bool hardcore, uint32_t participantID);
        static bool DownloadTransferItems(const std::string& modName, bool hardcore, uint32_t participantID, std::vector<uint8_t>* bytes = nullptr);

        static void RefreshCharacterMetadata(const std::wstring& playerName, uint32_t participantID, web::json::value& characterJSON);
        static void RefreshStashMetadata(const std::string& modName, bool hardcore, uint32_t participantID);

        static void WaitBackgroundComplete();

        // need to include function definition here because of template
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

        static uint32_t GetParticipantID(bool hardcore);
        static uint32_t GetCharacterID(const std::wstring& playerName, uint32_t participantID = 0);

        static FileMetadata GetServerCharacterMetadata(const std::wstring& playerName, uint32_t participantID);
        static FileMetadata GetServerStashMetadata(uint32_t participantID);

        static void OnDirectRead(void* data);
        static void OnWorldPreLoad(void* data);
        static void OnWorldUnload(void* data);
        static void OnSetMainPlayer(void* data);

        static void PostCharacterUpload(bool newPlayer = false);
        static void PostStashUpload();
        static void PostCloudStashUpload();
        static void PostPullTransferItems(const std::vector<Item*>& items);

        bool _characterTrusted;
        bool _stashTrusted;
        std::wstring _characterName;
        FileMetadata _characterMetadata;
        FileMetadata _lastTrustedCharacterMetadata;
        FileMetadata _stashMetadata;
        FileMetadata _lastTrustedStashMetadata;
        std::map<std::wstring, bool> _characterDownloadStatus;
        std::map<CharacterIDRef, uint32_t> _characterIDCache;
        concurrency::task_group _backgroundTasks;
};


#endif//INC_GDCL_DLL_SERVER_SYNC_H