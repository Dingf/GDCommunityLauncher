#include "ServerCache.h"

ServerCache::ServerCache()
{
    Reset();
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

    return 0;
}

uint32_t ServerCache::GetCharacterID(const std::wstring& playerName) const
{
    auto it = _characterIDCache.find(playerName);
    if (it != _characterIDCache.end())
        return it->second;

    // TODO: Get the character ID from the server and cache and return the result

    return 0;
}

void ServerCache::SetParticipantID(bool hardcore, uint32_t participantID)
{
    _participantIDCache[hardcore] = participantID;
}

void ServerCache::Reset()
{
    memset(_participantIDCache, 0, sizeof(uint32_t) * 2);
    _characterIDCache.clear();
}