#include "SeasonClient.h"
#include "ServerCache.h"
#include "ServerHandler.h"
#include "JSON.h"

ServerCache::ServerCache()
{
    Clear();
}

ServerCache* ServerCache::GetInstance()
{
    static ServerCache instance;
    return &instance;
}

uint32_t ServerCache::GetParticipantID(bool hardcore)
{
    if (_participantIDCache[hardcore] != 0)
        return _participantIDCache[hardcore];

    uint32_t seasonID = 0;
    for (const auto& season : spClient->GetSeasonList())
    {
        if ((1 + hardcore) == season._seasonType)
        {
            seasonID = season._seasonID;
            break;
        }
    }

    if (seasonID)
    {
        json result = spServer->Send("AddParticipant", seasonID).get();
        uint32_t participantID = result.at("SeasonParticipantId").get<uint32_t>();

        _participantIDCache[hardcore] = participantID;

        return participantID;
    }

    return 0;
}

uint32_t ServerCache::GetCharacterID(uint32_t participantID, const std::wstring& playerName)
{
    auto it = _characterIDCache.find(playerName);
    if (it != _characterIDCache.end())
        return it->second;

    json result = spServer->Send("GetCharacterData", participantID, playerName).get();
    uint32_t characterID = result.at("ParticipantCharacterId").get<uint32_t>();
    std::string characterChecksum = result.at("LastChecksum").get<std::string>();

    _characterIDCache[playerName] = characterID;

    // TODO: Also cache the last checksum value

    return characterID;
}

void ServerCache::Clear()
{
    memset(_participantIDCache, 0, sizeof(uint32_t) * 2);
    _characterIDCache.clear();
}