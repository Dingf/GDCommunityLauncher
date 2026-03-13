#ifndef INC_GDCL_DLL_SERVER_CACHE_H
#define INC_GDCL_DLL_SERVER_CACHE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "GameAPI/Difficulty.h"
#include "FileWriter.h"

// TODO: Make sure to make this thread-safe

// Things that we need to cache:
// Character data (per character)
// Quest data (per character/difficulty)
// Conversation data (per character/difficulty)
// FOW data (per character/difficulty)
// Stash data (hardcore/not hardcore)
// Character IDs
// Participant IDs


/*std::shared_ptr<CharacterBuffer>  _cachedCharacterBuffer;
std::shared_ptr<PlayerDataBuffer> _cachedQuestBuffer;
std::shared_ptr<PlayerDataBuffer> _cachedConversationsBuffer;
std::shared_ptr<PlayerDataBuffer> _cachedFOWBuffer;
std::shared_ptr<StashBuffer>      _cachedStashBuffer;
std::map<std::wstring, uint32_t> _characterIDCache;
std::map<bool, uint32_t> _participantIDCache;*/

class ServerCache
{
    public:
        struct CacheBuffer : public FileWriter
        {
            CacheBuffer(uint8_t* buffer, size_t size);

            uint32_t _participantID;
            std::string _checksum;
        };

        ServerCache(ServerCache&) = delete;
        void operator=(const ServerCache&) = delete;

        static ServerCache* GetInstance();

        bool IsParticipantHardcore(uint32_t participantID) const;

        uint32_t GetParticipantID(bool hardcore);
        uint32_t GetParticipantID(const std::wstring& playerName);

        const CacheBuffer* GetCharacterData(const std::wstring& playerName);
        const CacheBuffer* GetQuestData(const std::wstring& playerName, GameAPI::Difficulty difficulty);
        const CacheBuffer* GetConversationsData(const std::wstring& playerName, GameAPI::Difficulty difficulty);
        const CacheBuffer* GetMapData(const std::wstring& playerName, GameAPI::Difficulty difficulty);
        const CacheBuffer* GetFOWData(const std::wstring& playerName, GameAPI::Difficulty difficulty);

        void Clear();

    private:
        struct CacheDifficultySet
        {
            std::unique_ptr<CacheBuffer> _buffers[GameAPI::GAME_DIFFICULTY_MAX];
        };

        ServerCache();

        uint32_t _participantID[2];
        std::unordered_map<std::wstring, std::unique_ptr<CacheBuffer>> _characterData;
        std::unordered_map<std::wstring, CacheDifficultySet> _questData;
        std::unordered_map<std::wstring, CacheDifficultySet> _conversationsData;
        std::unordered_map<std::wstring, CacheDifficultySet> _mapData;
        std::unordered_map<std::wstring, CacheDifficultySet> _FOWData;

};

#define spCache ServerCache::GetInstance()

#endif//INC_GDCL_DLL_SERVER_CACHE_H