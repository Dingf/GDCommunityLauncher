#include "ServerCache.h"

ServerCache::ServerCache()
{
    ClearCache();
}

ServerCache* ServerCache::GetInstance()
{
    static ServerCache instance;
    return &instance;
}

uint32_t ServerCache::GetParticipantID(bool hardcore) const
{
    if (_participantIDCache[hardcore] != 0)
        return _participantIDCache[hardcore];

    // TODO: Get the participant ID from the server and cache and return the result
}

uint32_t ServerCache::GetCharacterID(const std::wstring& playerName) const
{
    if (_characterIDCache.count(playerName) > 0)
        return _characterIDCache[playerName];

    // TODO: Get the character ID from the server and cache and return the result
}

void ServerCache::Reset()
{
    memset(_participantIDCache, 0, sizeof(uint32_t) * 2);
}