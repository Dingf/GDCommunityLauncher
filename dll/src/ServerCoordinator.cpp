#include <unordered_map>
#include <filesystem>
#include "GameAPI/Difficulty.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ServerCoordinator.h"
#include "ServerCache.h"
#include "Character.h"
#include "MD5.h"
#include "Log.h"

ServerCoordinator::ServerCoordinator()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,              &ServerCoordinator::OnShutdownEvent);
        EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_READ,      &ServerCoordinator::OnDirectReadEvent);
        EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,        &ServerCoordinator::OnWorldPreLoadEvent);
    }
}

ServerCoordinator* ServerCoordinator::GetInstance()
{
    static ServerCoordinator instance;
    return &instance;
}

bool ServerCoordinator::Initialize()
{
    try
    {
        GetInstance();
        return true;
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ServerCoordinator module: %", ex.what());
        return false;
    }
}

void ServerCoordinator::OnShutdownEvent()
{
    // TODO: Upload cached buffers here
}

void LoadCharacterData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    Character character;
    if (character.ReadFromFile(filePath, true))
    {
        const std::wstring& playerName = character._headerBlock._charName;
        if (const FileWriter* cacheData = spCache->GetCharacterData(playerName))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

inline bool ParseCharacterFilePath(const std::filesystem::path& filePath, std::wstring& playerName, GameAPI::Difficulty& difficulty)
{
    // The file path is formatted as such: _<playerName>\<mapName>\<difficulty>\<filename>
    auto it = --filePath.end();                                             // Last element, e.g. the filename
    difficulty = GameAPI::GetGameDifficultyByName((--it)->stem().string()); // Difficulty
    playerName = (----it)->stem().wstring().substr(1);                      // Player name (skip the map name and trim the "_" prefix)

    if (difficulty == GameAPI::GAME_DIFFICULTY_UNKNOWN)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to parse game difficulty from path \"%\"", filePath.string());
        return false;
    }
    return true;
}

void LoadQuestData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring playerName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, playerName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetQuestData(playerName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

void LoadConversationsData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring playerName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, playerName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetConversationsData(playerName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

void LoadMapData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring playerName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, playerName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetMapData(playerName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

void LoadFOWData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring playerName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, playerName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetFOWData(playerName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

void LoadFormulasData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    // TODO
}

void LoadTransmutesData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    // TODO
}

typedef void (*DirectReadHandler)(const std::filesystem::path&, void**, size_t*);
const std::unordered_map<std::string, DirectReadHandler> _directReadHandlers =
{
    { "player.gdc",        LoadCharacterData },
    { "quests.gdd",        LoadQuestData },
    { "conversations.gdd", LoadConversationsData },
    { "map.dat",           LoadMapData },
    { "map.fow",           LoadFOWData },
    { "formulas.gst",      LoadFormulasData },
    { "formulas.gsh",      LoadFormulasData },
    { "transmutes.gst",    LoadTransmutesData },
    { "transmutes.gsh",    LoadTransmutesData },
};

void ServerCoordinator::OnDirectReadEvent(std::string filename, void** data, size_t* size)
{
    std::filesystem::path filePath = filename;
    if (std::filesystem::is_regular_file(filePath))
    {
        auto it = _directReadHandlers.find(filePath.filename().string());
        if (it != _directReadHandlers.end())
            it->second(filePath, data, size);
    }
}

void ServerCoordinator::OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded)
{
    if (mapName.starts_with("levels/mainmenu/"))
    {
        // TODO: Download all of the files here before the main menu loads

        /*std::vector<pplx::task<void>> downloadTasks;
        std::unordered_set<std::wstring> characterList;
        if (uint32_t softcoreID = GetInstance().GetParticipantID(false))
        {
            GetInstance().DownloadCharacterList(downloadTasks, softcoreID, characterList);
            GetInstance().DownloadFormulasFile(downloadTasks, softcoreID, false);
            GetInstance().DownloadTransmutesFile(downloadTasks, softcoreID, false);
        }

        if (uint32_t hardcoreID = GetInstance().GetParticipantID(true))
        {
            GetInstance().DownloadCharacterList(downloadTasks, hardcoreID, characterList);
            GetInstance().DownloadFormulasFile(downloadTasks, hardcoreID, true);
            GetInstance().DownloadTransmutesFile(downloadTasks, hardcoreID, true);
        }
        pplx::when_all(downloadTasks.begin(), downloadTasks.end()).wait();
        GetInstance().CleanupSaveFolder(characterList);*/
    }
}