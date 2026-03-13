#include "ServerCache.h"
#include "ServerHandler.h"
#include "DllClient.h"
#include "JSON.h"
#include "MD5.h"

ServerCache::CacheBuffer::CacheBuffer(uint8_t* buffer, size_t size) : FileWriter(buffer, size)
{
    _participantID = 0;
    _checksum = GenerateBufferMD5(buffer, size);
}

ServerCache::ServerCache()
{
    Clear();
}

ServerCache* ServerCache::GetInstance()
{
    static ServerCache instance;
    return &instance;
}

bool ServerCache::IsParticipantHardcore(uint32_t participantID) const
{
    // This code assumes that if you have the participant ID, then it's already been cached earlier
    // Otherwise, a participant ID that doesn't match either will return false (which shouldn't happen)
    return (_participantID[1] == participantID);
}

uint32_t ServerCache::GetParticipantID(bool hardcore)
{
    if (_participantID[hardcore] != 0)
        return _participantID[hardcore];

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

            _participantID[hardcore] = participantID;

            return participantID;
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "AddParticipant request to server timed out.");
        }
    }

    return 0;
}

uint32_t ServerCache::GetParticipantID(const std::wstring& playerName)
{
    if (const CacheBuffer* buffer = GetCharacterData(playerName))
    {
        return buffer->_participantID;
    }
    return 0;
}

const ServerCache::CacheBuffer* ServerCache::GetCharacterData(const std::wstring& playerName)
{
    auto it = _characterData.find(playerName);
    return (it != _characterData.end()) ? it->second.get() : nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetQuestData(const std::wstring& playerName, GameAPI::Difficulty difficulty)
{
    auto it = _questData.find(playerName);
    if ((it != _questData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        return it->second._buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetConversationsData(const std::wstring& playerName, GameAPI::Difficulty difficulty)
{
    auto it = _conversationsData.find(playerName);
    if ((it != _conversationsData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        return it->second._buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetMapData(const std::wstring& playerName, GameAPI::Difficulty difficulty)
{
    auto it = _mapData.find(playerName);
    if ((it != _mapData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        return it->second._buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetFOWData(const std::wstring& playerName, GameAPI::Difficulty difficulty)
{
    auto it = _FOWData.find(playerName);
    if ((it != _FOWData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        return it->second._buffers[difficulty].get();
    }
    return nullptr;
}

/*uint32_t ServerCache::GetCharacterID(uint32_t participantID, const std::wstring& playerName)
{
    auto it = _characterIDs.find(playerName);
    if (it != _characterIDs.end())
        return it->second;

    auto future = spServer->Send("GetCharacterData", participantID, playerName);
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(5);
    if (future.wait_until(timeout) == std::future_status::ready)
    {
        json result = future.get();
        uint32_t characterID = result.at("ParticipantCharacterId").get<uint32_t>();
        std::string characterChecksum = result.at("LastChecksum").get<std::string>();

        _characterIDs[playerName] = characterID;

        // TODO: Also cache the last checksum value

        return characterID;
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "GetCharacterData request to server timed out.");
    }

    return 0;
}*/

void ServerCache::Clear()
{
    memset(_participantID, 0, sizeof(uint32_t) * 2);
    _characterData.clear();
    _questData.clear();
    _conversationsData.clear();
    _mapData.clear();
    _FOWData.clear();
}