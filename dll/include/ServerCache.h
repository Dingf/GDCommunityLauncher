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

            std::string _checksum;
        };

        ServerCache(ServerCache&) = delete;
        void operator=(const ServerCache&) = delete;

        static ServerCache* GetInstance();

        bool IsParticipantHardcore(uint32_t participantID) const;

        uint32_t GetParticipantID(bool hardcore);
        uint32_t GetParticipantID(const std::wstring& characterName);
        uint32_t GetCharacterID(const std::wstring& characterName);
        int32_t GetStashCapacity() const { return _stashCapacity; }

        void SetParticipantID(bool hardcore, uint32_t participantID);
        void SetStashCapacity(int32_t capacity) { _stashCapacity = capacity; }

        const CacheBuffer* GetCharacterData(const std::wstring& characterName) const;
        const CacheBuffer* GetQuestData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const;
        const CacheBuffer* GetConversationsData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const;
        const CacheBuffer* GetMapData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const;
        const CacheBuffer* GetFOWData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const;

        const CacheBuffer* GetStashData(bool hardcore) const;
        const CacheBuffer* GetFormulasData(bool hardcore) const;
        const CacheBuffer* GetTransmutesData(bool hardcore) const;
        const CacheBuffer* GetTagsData(bool hardcore) const;

        void SetCharacterID(const std::wstring& characterName, uint32_t participantID, uint32_t characterID);
        void SetCharacterData(const std::wstring& characterName, uint8_t* data, size_t size);
        void SetQuestData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size);
        void SetConversationsData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size);
        void SetMapData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size);
        void SetFOWData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size);

        void SetStashData(bool hardcore, uint8_t* data, size_t size);
        void SetFormulasData(bool hardcore, uint8_t* data, size_t size);
        void SetTransmutesData(bool hardcore, uint8_t* data, size_t size);
        void SetTagsData(bool hardcore, uint8_t* data, size_t size);

        void Clear();

    private:
        struct CacheDifficultySet
        {
            std::unique_ptr<CacheBuffer> _buffers[GameAPI::GAME_DIFFICULTY_MAX];
        };

        struct CharacterData
        {
            CharacterData() : _characterID(0), _participantID(0)
            {
                _quests        = std::make_unique<CacheDifficultySet>();
                _conversations = std::make_unique<CacheDifficultySet>();
                _map           = std::make_unique<CacheDifficultySet>();
                _FOW           = std::make_unique<CacheDifficultySet>();
            }

            uint32_t                            _characterID;
            uint32_t                            _participantID;
            std::unique_ptr<CacheBuffer>        _character;
            std::unique_ptr<CacheDifficultySet> _quests;
            std::unique_ptr<CacheDifficultySet> _conversations;
            std::unique_ptr<CacheDifficultySet> _map;
            std::unique_ptr<CacheDifficultySet> _FOW;
        };

        struct ParticipantData
        {
            ParticipantData() : _participantID(0) {}

            uint32_t                     _participantID;
            std::unique_ptr<CacheBuffer> _stash;
            std::unique_ptr<CacheBuffer> _formulas;
            std::unique_ptr<CacheBuffer> _transmutes;
            std::unique_ptr<CacheBuffer> _tags;
        };

        ServerCache();

        int32_t _stashCapacity;
        std::unordered_map<bool, ParticipantData> _participantData;
        std::unordered_map<std::wstring, CharacterData> _characterData;

};

#define spCache ServerCache::GetInstance()

#endif//INC_GDCL_DLL_SERVER_CACHE_H