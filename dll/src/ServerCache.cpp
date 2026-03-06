#include "ServerCache.h"
#include "ServerHandler.h"
#include "JSON.h"
#include "DllClient.h"

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
        auto future = spServer->Send("AddParticipant", seasonID);
        auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(5);
        if (future.wait_until(timeout) == std::future_status::ready)
        {
            json result = future.get();
            uint32_t participantID = result.at("SeasonParticipantId").get<uint32_t>();

            _participantIDCache[hardcore] = participantID;

            return participantID;
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "AddParticipant request to server timed out.");
        }
    }

    return 0;
}

uint32_t ServerCache::GetCharacterID(uint32_t participantID, const std::wstring& playerName)
{
    auto it = _characterIDCache.find(playerName);
    if (it != _characterIDCache.end())
        return it->second;

    auto future = spServer->Send("GetCharacterData", participantID, playerName);
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(5);
    if (future.wait_until(timeout) == std::future_status::ready)
    {
        json result = future.get();
        uint32_t characterID = result.at("ParticipantCharacterId").get<uint32_t>();
        std::string characterChecksum = result.at("LastChecksum").get<std::string>();

        _characterIDCache[playerName] = characterID;

        // TODO: Also cache the last checksum value

        return characterID;
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "GetCharacterData request to server timed out.");
    }

    return 0;
}

void ServerCache::Clear()
{
    memset(_participantIDCache, 0, sizeof(uint32_t) * 2);
    _characterIDCache.clear();
}