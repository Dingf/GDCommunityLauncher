#include "ServerCache.h"
#include "DllClient.h"
#include "JSON.h"
#include "MD5.h"

ServerCache::CacheBuffer::CacheBuffer(uint8_t* buffer, size_t size) : FileWriter(buffer, size)
{
    _checksum = GenerateBufferMD5(buffer, size);
}

ServerCache::ServerCache() : _stashCapacity(0)
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
    auto it = _participantData.find(true);
    if (it != _participantData.end())
    {
        return it->second._participantID == participantID;
    }
    return false;
}

bool ServerCache::HasCharacterData(const std::wstring& characterName) const
{
    return _characterData.contains(characterName);
}

uint32_t ServerCache::GetParticipantID(bool hardcore)
{
    auto it = _participantData.find(hardcore);
    return (it != _participantData.end()) ? it->second._participantID : 0;
}

uint32_t ServerCache::GetParticipantID(const std::wstring& characterName)
{
    auto it = _characterData.find(characterName);
    return (it != _characterData.end()) ? it->second._participantID : 0;
}

uint32_t ServerCache::GetCharacterID(const std::wstring& characterName)
{
    auto it = _characterData.find(characterName);
    return (it != _characterData.end()) ? it->second._characterID : 0;
}

void ServerCache::SetParticipantID(bool hardcore, uint32_t participantID)
{
    _participantData[hardcore]._participantID = participantID;
}

const ServerCache::CacheBuffer* ServerCache::GetCharacterData(const std::wstring& characterName) const
{
    auto it = _characterData.find(characterName);
    if (it != _characterData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._character.get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetQuestData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const
{
    auto it = _characterData.find(characterName);
    if ((it != _characterData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._quests->_buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetConversationsData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const
{
    auto it = _characterData.find(characterName);
    if ((it != _characterData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._conversations->_buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetMapData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const
{
    auto it = _characterData.find(characterName);
    if ((it != _characterData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._map->_buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetFOWData(const std::wstring& characterName, GameAPI::Difficulty difficulty) const
{
    auto it = _characterData.find(characterName);
    if ((it != _characterData.end()) && (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN))
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._FOW->_buffers[difficulty].get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetStashData(bool hardcore) const
{
    auto it = _participantData.find(hardcore);
    if (it != _participantData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._stash.get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetFormulasData(bool hardcore) const
{
    auto it = _participantData.find(hardcore);
    if (it != _participantData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._formulas.get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetTransmutesData(bool hardcore) const
{
    auto it = _participantData.find(hardcore);
    if (it != _participantData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._transmutes.get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetReagentsData(bool hardcore) const
{
    auto it = _participantData.find(hardcore);
    if (it != _participantData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._reagents.get();
    }
    return nullptr;
}

const ServerCache::CacheBuffer* ServerCache::GetTagsData(bool hardcore) const
{
    auto it = _participantData.find(hardcore);
    if (it != _participantData.end())
    {
        std::lock_guard<std::mutex> lock(it->second._mutex);
        return it->second._tags.get();
    }
    return nullptr;
}

void ServerCache::SetCharacterID(const std::wstring& characterName, uint32_t participantID, uint32_t characterID)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    _characterData[characterName]._participantID = participantID;
    _characterData[characterName]._characterID = characterID;
}

void ServerCache::SetCharacterData(const std::wstring& characterName, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    _characterData[characterName]._character = std::make_unique<CacheBuffer>(data, size);
    _dirtyCharacters.insert(characterName);
}

void ServerCache::SetQuestData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    if (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN)
        _characterData[characterName]._quests->_buffers[difficulty] = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetConversationsData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    if (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN)
        _characterData[characterName]._conversations->_buffers[difficulty] = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetMapData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    if (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN)
        _characterData[characterName]._map->_buffers[difficulty] = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetFOWData(const std::wstring& characterName, GameAPI::Difficulty difficulty, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_characterData[characterName]._mutex);
    if (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN)
        _characterData[characterName]._FOW->_buffers[difficulty] = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetStashData(bool hardcore, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_participantData[hardcore]._mutex);
    _participantData[hardcore]._stash = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetFormulasData(bool hardcore, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_participantData[hardcore]._mutex);
    _participantData[hardcore]._formulas = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetTransmutesData(bool hardcore, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_participantData[hardcore]._mutex);
    _participantData[hardcore]._transmutes = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetReagentsData(bool hardcore, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_participantData[hardcore]._mutex);
    _participantData[hardcore]._reagents = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::SetTagsData(bool hardcore, uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(_participantData[hardcore]._mutex);
    _participantData[hardcore]._tags = std::make_unique<CacheBuffer>(data, size);
}

void ServerCache::Clear()
{
    _stashCapacity = -1;
    _participantData.clear();
    _characterData.clear();
}