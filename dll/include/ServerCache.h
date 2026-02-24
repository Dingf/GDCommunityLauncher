#ifndef INC_GDCL_DLL_SERVER_CACHE_H
#define INC_GDCL_DLL_SERVER_CACHE_H

#include <string>
#include <vector>
#include <unordered_map>

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
        ServerCache(ServerCache&) = delete;
        void operator=(const ServerCache&) = delete;

        static ServerCache* GetInstance();

        uint32_t GetParticipantID(bool hardcore);
        uint32_t GetCharacterID(uint32_t participantID, const std::wstring& playerName);

        void Clear();

    private:
        ServerCache();

        uint32_t _participantIDCache[2];
        std::unordered_map<std::wstring, uint32_t> _characterIDCache;


};

#define spCache ServerCache::GetInstance()

#endif//INC_GDCL_DLL_SERVER_CACHE_H