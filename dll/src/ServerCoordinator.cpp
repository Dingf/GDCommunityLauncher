#include <unordered_map>
#include <filesystem>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatAPI.h"
#include "ChatHandler.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ServerCoordinator.h"
#include "ServerCache.h"
#include "ServerHandler.h"
#include "Character.h"
#include "Quest.h"
#include "FileReader.h"
#include "StringConvert.h"
#include "HTTP.h"
#include "MD5.h"
#include "Log.h"

typedef ThreadSafeQueue<ServerHandlerRequest> DownloadQueue;

ServerCoordinator::ServerCoordinator()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_UPDATE,             OnUpdateEvent);
        EventManager::Subscribe(GDCL_EVENT_PRE_SHUTDOWN,       OnPreShutdownEvent);
        EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_READ,   OnDirectReadEvent);
        EventManager::Subscribe(GDCL_EVENT_DIRECT_FILE_WRITE,  OnDirectWriteEvent);
        EventManager::Subscribe(GDCL_EVENT_ADD_SAVE_JOB,       OnAddSaveJobEvent);
        EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,     OnWorldPreLoadEvent);
        EventManager::Subscribe(GDCL_EVENT_SET_SEASON_PLAYER,  OnSetSeasonPlayerEvent);
        EventManager::Subscribe(GDCL_EVENT_CARAVAN_INTERACT,   OnCaravanInteractEvent);
        EventManager::Subscribe(GDCL_EVENT_TRANSFER_PRE_SAVE,  OnTransferPreSaveEvent);
        EventManager::Subscribe(GDCL_EVENT_DELETE_FILE,        OnDeleteFileEvent);
        EventManager::Subscribe(GDCL_EVENT_BESTOW_TOKEN,       OnBestowTokenEvent);
    }
}

ServerCoordinator* ServerCoordinator::GetInstance()
{
    static ServerCoordinator instance;
    return &instance;
}

void ServerCoordinator::OnUpdateEvent()
{
    const auto& dirtyCharacters = spCache->GetDirtyCharacters();
    if (!dirtyCharacters.empty())
    {
        for (const std::wstring& characterName : dirtyCharacters)
        {
            if (const FileWriter* buffer = spCache->GetCharacterData(characterName))
            {
                uint32_t participantID = spCache->GetParticipantID(characterName);
                spServer->Send("SaveCharacterFile", participantID, characterName, BinaryToBase64(buffer->GetBuffer(), buffer->GetBufferSize()));
            }
        }
        spCache->ClearDirtyCharacters();
    }

    uint32_t caravanID = GameAPI::GetLastCaravanID();
    if (caravanID != 0)
    {
        if (void* caravan = EngineAPI::FindObjectByID(caravanID))
        {
            std::string caravanTag = *(std::string*)((uint8_t*)caravan + 0x440);
            GameAPI::SetCaravanDriverTag(caravanTag);
            GameAPI::DisplayCaravanWindow(caravanID);
        }
        GameAPI::SetLastCaravanID(0);
    }
}

void ServerCoordinator::OnPreShutdownEvent()
{
    std::wstring lastPlayerName = spClient->GetMainPlayerName();
    if (const FileWriter* characterData = spCache->GetCharacterData(lastPlayerName))
    {
        uint32_t participantID = spCache->GetParticipantID(lastPlayerName);
        spServer->Send("SaveCharacterFile", participantID, lastPlayerName, BinaryToBase64(characterData->GetBuffer(), characterData->GetBufferSize()));
    }
}

static void LoadCharacterData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    Character character;
    if (character.ReadFromFile(filePath, true))
    {
        const std::wstring& characterName = character._headerBlock._charName;
        if (const FileWriter* cacheData = spCache->GetCharacterData(characterName))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

static inline bool ParseCharacterFilePath(const std::filesystem::path& filePath, std::wstring& characterName, GameAPI::Difficulty& difficulty)
{
    // The file path is formatted as such: _<characterName>\<mapName>\<difficulty>\<filename>
    auto it = --filePath.end();                                             // Last element, e.g. the filename
    difficulty = GameAPI::GetGameDifficultyByName((--it)->stem().string()); // Difficulty
    characterName = (----it)->stem().wstring().substr(1);                   // Character name (skip the map name and trim the "_" prefix)

    return (difficulty != GameAPI::GAME_DIFFICULTY_UNKNOWN);
}

static void LoadQuestData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, characterName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetQuestData(characterName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

static void LoadConversationsData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, characterName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetConversationsData(characterName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

static void LoadMapData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, characterName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetMapData(characterName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

static void LoadFOWData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if (ParseCharacterFilePath(filePath, characterName, difficulty))
    {
        if (const FileWriter* cacheData = spCache->GetFOWData(characterName, difficulty))
        {
            *size = cacheData->GetBufferSize();
            *data = new uint8_t[*size];
            memcpy(*data, cacheData->GetBuffer(), *size);
        }
    }
}

// map.dat and map.fow have the same stem, so choose based on the extension
static inline void LoadMapOrFOWData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    std::filesystem::path extension = filePath.extension();
    if (extension == ".dat")
        LoadMapData(filePath, data, size);
    else if (extension == ".fow")
        LoadFOWData(filePath, data, size);
    else
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load unsupported map file %", filePath.filename().string());
}

static void LoadFormulasData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (const FileWriter* cacheData = spCache->GetFormulasData(hardcore))
    {
        *size = cacheData->GetBufferSize();
        *data = new uint8_t[*size];
        memcpy(*data, cacheData->GetBuffer(), *size);
    }
}

static void LoadTransmutesData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (const FileWriter* cacheData = spCache->GetTransmutesData(hardcore))
    {
        *size = cacheData->GetBufferSize();
        *data = new uint8_t[*size];
        memcpy(*data, cacheData->GetBuffer(), *size);
    }
}

static void LoadStashData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (const FileWriter* cacheData = spCache->GetStashData(hardcore))
    {
        *size = cacheData->GetBufferSize();
        *data = new uint8_t[*size];
        memcpy(*data, cacheData->GetBuffer(), *size);
    }
}

static void LoadReagentsData(const std::filesystem::path& filePath, void** data, size_t* size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (const FileWriter* cacheData = spCache->GetReagentsData(hardcore))
    {
        *size = cacheData->GetBufferSize();
        *data = new uint8_t[*size];
        memcpy(*data, cacheData->GetBuffer(), *size);
    }
}

void ServerCoordinator::OnDirectReadEvent(std::string filename, void** data, size_t* size)
{
    typedef void (*DirectReadHandler)(const std::filesystem::path&, void**, size_t*);
    static const std::unordered_map<std::string, DirectReadHandler> directReadHandlers =
    {
        { "player",        LoadCharacterData },
        { "quests",        LoadQuestData },
        { "conversations", LoadConversationsData },
        { "map",           LoadMapOrFOWData },
        { "formulas",      LoadFormulasData },
        { "transmutes",    LoadTransmutesData },
        { "transfer",      LoadStashData },
        { "reagents",      LoadReagentsData },
    };

    std::filesystem::path filePath = filename;
    if (std::filesystem::is_regular_file(filePath))
    {
        auto it = directReadHandlers.find(filePath.stem().string());
        if (it != directReadHandlers.end())
            it->second(filePath, data, size);
    }
}

static void SaveStashData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (uint32_t participantID = spCache->GetParticipantID(hardcore))
    {
        spCache->SetStashData(hardcore, data, size);
        spServer->Send("SaveParticipantStashFile", participantID, BinaryToBase64(data, size));
    }
}

static void SaveFormulasData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (uint32_t participantID = spCache->GetParticipantID(hardcore))
    {
        spCache->SetFormulasData(hardcore, data, size);
        spServer->Send("SaveParticipantFormulas", participantID, BinaryToBase64(data, size));
    }
}

static void SaveTransmutesData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (uint32_t participantID = spCache->GetParticipantID(hardcore))
    {
        spCache->SetTransmutesData(hardcore, data, size);
        spServer->Send("SaveParticipantTransmutes", participantID, BinaryToBase64(data, size));
    }
}

static void SaveReagentsData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    bool hardcore = filePath.extension().string().ends_with("sh");
    if (uint32_t participantID = spCache->GetParticipantID(hardcore))
    {
        spCache->SetReagentsData(hardcore, data, size);
        spServer->Send("SaveParticipantReagentsFile", participantID, BinaryToBase64(data, size));
    }
}

void ServerCoordinator::OnDirectWriteEvent(std::string filename, void* data, size_t size)
{
    typedef void (*DirectWriteHandler)(const std::filesystem::path&, uint8_t*, size_t);
    static const std::unordered_map<std::string, DirectWriteHandler> directWriteHandlers =
    {
        { "transfer",   SaveStashData },
        { "formulas",   SaveFormulasData },
        { "transmutes", SaveTransmutesData },
        { "reagents",   SaveReagentsData }
    };

    std::filesystem::path filePath = filename;
    auto it = directWriteHandlers.find(filePath.stem().string());
    if (it != directWriteHandlers.end())
        it->second(filePath, (uint8_t*)data, size);
}

static void SaveCharacterData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    Character character;
    if (character.ReadFromBuffer(data, size, true))
    {
        const std::wstring& characterName = character._headerBlock._charName;
        uint32_t participantID = spCache->GetParticipantID(character._headerBlock._charIsHardcore);
        spCache->SetCharacterData(characterName, data, size);

        // Set the participant ID if it's a new character
        if (!std::filesystem::exists(filePath))
            spCache->SetCharacterID(characterName, participantID, 0);
    }
}

static void SaveTagsFile(uint32_t participantID)
{
    bool hardcore = EngineAPI::IsHardcore();
    std::string filename = hardcore ? "tags.gsh" : "tags.gst";
    std::filesystem::path filePath = GameAPI::GetUserSaveFolder() / filename;

    void* mainPlayer = GameAPI::GetMainPlayer();
    if ((mainPlayer) && (spClient->IsPlayingSeason()))
    {
        const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, GameAPI::GetGameDifficulty());

        uint32_t count = 0;
        std::string buffer;
        for (GameAPI::TriggerToken token : tokens)
        {
            std::string tokenString = token;
            if (tokenString.starts_with("GDIP_"))
            {
                uint32_t tokenSize = tokenString.size();
                buffer.append((const char*)&tokenSize, 4);
                buffer += tokenString;
                count++;
            }
        }

        size_t fileSize = 8 + buffer.size();
        FileWriter writer(fileSize);

        writer.BufferInt32(count);
        writer.BufferString(buffer);
        writer.WriteToFile(filePath);

        spCache->SetTagsData(hardcore, writer.GetBuffer(), writer.GetBufferSize());
        spServer->Send("SaveParticipantTagFile", participantID, BinaryToBase64(writer.GetBuffer(), writer.GetBufferSize()));
    }
}

static void SaveQuestData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if ((EngineAPI::IsMainCampaign()) && (ParseCharacterFilePath(filePath, characterName, difficulty)))
    {
        if (uint32_t participantID = spCache->GetParticipantID(characterName))
        {
            spCache->SetQuestData(characterName, difficulty, data, size);
            spServer->Send("SaveParticipantCharacterQuestFile", participantID, characterName, difficulty, BinaryToBase64(data, size));

            // Only save tags file if the character already exists, so that we don't overwrite all of them on a new character
            if (std::filesystem::exists(filePath))
                SaveTagsFile(participantID);
        }
    }
}

static void SaveConversationsData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if ((EngineAPI::IsMainCampaign()) && (ParseCharacterFilePath(filePath, characterName, difficulty)))
    {
        if (uint32_t participantID = spCache->GetParticipantID(characterName))
        {
            spCache->SetConversationsData(characterName, difficulty, data, size);
            spServer->Send("SaveParticipantCharacterConversationsFile", participantID, characterName, difficulty, BinaryToBase64(data, size));
        }
    }
}

static void SaveMapData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if ((EngineAPI::IsMainCampaign()) && (ParseCharacterFilePath(filePath, characterName, difficulty)))
    {
        if (uint32_t participantID = spCache->GetParticipantID(characterName))
        {
            spCache->SetMapData(characterName, difficulty, data, size);
            spServer->Send("SaveParticipantCharacterMapDatFile", participantID, characterName, difficulty, BinaryToBase64(data, size));
        }
    }
}

static void SaveFOWData(const std::filesystem::path& filePath, uint8_t* data, size_t size)
{
    std::wstring characterName;
    GameAPI::Difficulty difficulty;

    if ((EngineAPI::IsMainCampaign()) && (ParseCharacterFilePath(filePath, characterName, difficulty)))
    {
        if (uint32_t participantID = spCache->GetParticipantID(characterName))
        {
            spCache->SetFOWData(characterName, difficulty, data, size);
            spServer->Send("SaveParticipantCharacterMapFowFile", participantID, characterName, difficulty, BinaryToBase64(data, size));
        }
    }
}

void ServerCoordinator::OnAddSaveJobEvent(std::string filename, void* data, size_t size)
{
    typedef void (*SaveJobHandler)(const std::filesystem::path&, uint8_t*, size_t);
    static const std::unordered_map<std::string, SaveJobHandler> saveJobHandlers =
    {
        { "player.gdc",        SaveCharacterData },
        { "quests.gdd",        SaveQuestData },
        { "conversations.gdd", SaveConversationsData },
        { "map.dat",           SaveMapData },
        { "map.fow",           SaveFOWData },
    };

    std::filesystem::path filePath = filename;
    auto it = saveJobHandlers.find(filePath.filename().string());
    if (it != saveJobHandlers.end())
        it->second(filePath, (uint8_t*)data, size);
}

static void DownloadCharacterFile(uint32_t participantID, const std::wstring& characterName, std::shared_ptr<DownloadQueue> downloadQueue)
{
    std::filesystem::path filePath = GameAPI::GetPlayerSaveFile(characterName);
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, characterName, GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetCharacterData(characterName, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetCharacterFile", participantID, characterName));
    }));
}

static void DownloadQuestFile(uint32_t participantID, const std::wstring& characterName, GameAPI::Difficulty difficulty, std::shared_ptr<DownloadQueue> downloadQueue)
{
    std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "quests.gdd";
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, characterName, difficulty, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetQuestData(characterName, difficulty, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantCharacterQuestFile", participantID, characterName, difficulty));
    }));
}

static void DownloadConversationsFile(uint32_t participantID, const std::wstring& characterName, GameAPI::Difficulty difficulty, std::shared_ptr<DownloadQueue> downloadQueue)
{
    std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "conversations.gdd";
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, characterName, difficulty, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetConversationsData(characterName, difficulty, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantCharacterConversationsFile", participantID, characterName, difficulty));
    }));
}

static void DownloadMapFile(uint32_t participantID, const std::wstring& characterName, GameAPI::Difficulty difficulty, std::shared_ptr<DownloadQueue> downloadQueue)
{
    std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "map.dat";
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, characterName, difficulty, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetMapData(characterName, difficulty, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantCharacterMapDatFile", participantID, characterName, difficulty));
    }));
}

static void DownloadFOWFile(uint32_t participantID, const std::wstring& characterName, GameAPI::Difficulty difficulty, std::shared_ptr<DownloadQueue> downloadQueue)
{
    std::filesystem::path filePath = GameAPI::GetPlayerFolder(characterName) / "levels_world001.map" / GameAPI::GetGameDifficultyName(difficulty) / "map.fow";
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, characterName, difficulty, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetFOWData(characterName, difficulty, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantCharacterMapFowFile", participantID, characterName, difficulty));
    }));
}

static void DownloadStashFile(uint32_t participantID, std::shared_ptr<DownloadQueue> downloadQueue)
{
    bool hardcore = spCache->IsParticipantHardcore(participantID);
    std::filesystem::path filePath = GameAPI::GetTransferStashPath(hardcore);
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, std::wstring(), GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetStashData(hardcore, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantStashFile", participantID));
    }));
}

static void DownloadFormulasFile(uint32_t participantID, std::shared_ptr<DownloadQueue> downloadQueue)
{
    bool hardcore = spCache->IsParticipantHardcore(participantID);
    std::filesystem::path filePath = GameAPI::GetFormulasPath(hardcore);
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, std::wstring(), GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetFormulasData(hardcore, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantFormulas", participantID));
    }));
}

static void DownloadTransmutesFile(uint32_t participantID, std::shared_ptr<DownloadQueue> downloadQueue)
{
    bool hardcore = spCache->IsParticipantHardcore(participantID);
    std::filesystem::path filePath = GameAPI::GetTransmutesPath(hardcore);
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, std::wstring(), GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetTransmutesData(hardcore, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantTransmutes", participantID));
    }));
}

static void DownloadTagsFile(uint32_t participantID, std::shared_ptr<DownloadQueue> downloadQueue)
{
    bool hardcore = spCache->IsParticipantHardcore(participantID);
    std::string filename = hardcore ? "tags.gsh" : "tags.gst";
    std::filesystem::path filePath = GameAPI::GetUserSaveFolder() / filename;
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, std::wstring(), GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetTagsData(hardcore, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantTagFile", participantID));
    }));
}

static void DownloadReagentsFile(uint32_t participantID, std::shared_ptr<DownloadQueue> downloadQueue)
{
    bool hardcore = spCache->IsParticipantHardcore(participantID);
    std::filesystem::path filePath = GameAPI::GetReagentsPath(hardcore);
    downloadQueue->push(spServer->Send("GetFileChecksum", participantID, std::wstring(), GameAPI::GAME_DIFFICULTY_NORMAL, filePath).then(
    [=](const json& response)
    {
        HTTPStatus status = response.at("StatusCode").get<HTTPStatus>();
        if (status == HTTP_STATUS_OK)
        {
            std::string serverChecksum = response.at("Data").get<std::string>();
            std::string clientChecksum = GenerateFileMD5(filePath);
            if (((serverChecksum == clientChecksum) && (!clientChecksum.empty())))
            {
                FileReader reader(filePath);
                spCache->SetReagentsData(hardcore, reader.GetBuffer(), reader.GetBufferSize());
                return;
            }
        }
        downloadQueue->push(spServer->Send("GetParticipantReagentsFile", participantID));
    }));
}

static void DownloadParticipantFiles(std::shared_ptr<DownloadQueue> downloadQueue, uint32_t participantID)
{
    json characters = spServer->Send("GetParticipantCharacters", participantID).get().at("Data");
    for (const json& character : characters)
    {
        std::wstring characterName = CharToWide(character.get<std::string>());

        if (!spCache->GetCharacterData(characterName))
        {
            downloadQueue->push(spServer->Send("GetCharacterData", participantID, characterName));
            DownloadCharacterFile(participantID, characterName, downloadQueue);
        }

        for (GameAPI::Difficulty difficulty : GameAPI::GAME_DIFFICULTIES)
        {
            if (!spCache->GetQuestData(characterName, difficulty))
                DownloadQuestFile(participantID, characterName, difficulty, downloadQueue);
            if (!spCache->GetConversationsData(characterName, difficulty))
                DownloadConversationsFile(participantID, characterName, difficulty, downloadQueue);
            if (!spCache->GetMapData(characterName, difficulty))
                DownloadMapFile(participantID, characterName, difficulty, downloadQueue);
            if (!spCache->GetFOWData(characterName, difficulty))
                DownloadFOWFile(participantID, characterName, difficulty, downloadQueue);
        }
    }

    bool hardcore = spCache->IsParticipantHardcore(participantID);
    if (!spCache->GetStashData(hardcore))
        DownloadStashFile(participantID, downloadQueue);
    if (!spCache->GetFormulasData(hardcore))
        DownloadFormulasFile(participantID, downloadQueue);
    if (!spCache->GetTransmutesData(hardcore))
        DownloadTransmutesFile(participantID, downloadQueue);
    if (!spCache->GetTagsData(hardcore))
        DownloadTagsFile(participantID, downloadQueue);
    if (!spCache->GetReagentsData(hardcore))
        DownloadReagentsFile(participantID, downloadQueue);
    if (!spCache->GetStashCapacity())
        downloadQueue->push(spServer->Send("GetParticipantSharedStashCapacity"));
}

static void CleanupSaveFolder()
{
    std::filesystem::path mainPath = GameAPI::GetUserSaveFolder() / "main";
    if (std::filesystem::is_directory(mainPath))
    {
        for (const auto& it : std::filesystem::directory_iterator(mainPath))
        {
            const std::filesystem::path& filePath = it.path();
            if (std::filesystem::is_directory(filePath))
            {
                std::wstring characterName = filePath.filename().wstring().substr(1);
                if (spCache->HasCharacterData(characterName))
                    continue;
            }
            std::filesystem::remove_all(filePath);
        }
    }
}

void ServerCoordinator::OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded)
{
    if (mapName.starts_with("levels/mainmenu/"))
    {
        try
        {
            std::shared_ptr<DownloadQueue> downloadQueue = std::make_shared<DownloadQueue>();

            uint32_t hardcoreID = spCache->GetParticipantID(true);
            if (!hardcoreID)
            {
                json result = spServer->Send("AddParticipant", true).get();
                hardcoreID = result.at("Data").at("SeasonParticipantId").get<uint32_t>();
                DownloadParticipantFiles(downloadQueue, hardcoreID);

                if (const SeasonInfo* seasonInfo = spClient->GetSeasonByType(true))
                {
                    downloadQueue->push(spServer->Send("GetSeasonChallenges", seasonInfo->_seasonID));
                }
            }

            uint32_t softcoreID = spCache->GetParticipantID(false);
            if (!softcoreID)
            {
                json result = spServer->Send("AddParticipant", false).get();
                softcoreID = result.at("Data").at("SeasonParticipantId").get<uint32_t>();
                DownloadParticipantFiles(downloadQueue, softcoreID);

                if (const SeasonInfo* seasonInfo = spClient->GetSeasonByType(false))
                {
                    downloadQueue->push(spServer->Send("GetSeasonChallenges", seasonInfo->_seasonID));
                }
            }

            while (!downloadQueue->empty())
            {
                ServerHandlerRequest task;
                if (downloadQueue->front(task) && task.valid())
                    task.wait();

                downloadQueue->pop();
            }

            CleanupSaveFolder();
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to retrieve player data from server: %", ex.what());
        }
    }
}

static void LoadQuestStatesForPlayer(void* player)
{
    GameAPI::Difficulty difficulty = GameAPI::GetGameDifficulty();
    std::wstring characterName = GameAPI::GetPlayerName(player);
    if (const auto* data = spCache->GetQuestData(characterName, difficulty))
    {
        Quest questData;
        if (questData.ReadFromBuffer(data->GetBuffer(), data->GetBufferSize()))
        {
            GameAPI::ClearPlayerTokens(player);
            for (const auto& token : questData._tokensBlock._questTokens)
            {
                // Skip loading these, as they will be loaded from the tags file
                if (!token.starts_with("GDIP_"))
                    GameAPI::BestowTokenNow(player, token);
            }

            for (const auto& questData : questData._dataBlock._questData)
            {
                void* quest = GameAPI::GetQuestByID(questData._id1);
                for (const auto& taskData : questData._tasks)
                {
                    if (void* task = GameAPI::GetQuestTaskByID(quest, taskData._id1))
                    {
                        GameAPI::SetQuestTaskState(task, taskData._state);
                        GameAPI::SetQuestTaskInProgress(task, taskData._isInProgress);
                    }
                }
            }
        }
    }
}

static void LoadSeasonTagsForPlayer(void* player)
{
    std::wstring characterName = GameAPI::GetPlayerName(player);
    if (uint32_t participantID = spCache->GetParticipantID(characterName))
    {
        bool hardcore = spCache->IsParticipantHardcore(participantID);
        if (const auto* data = spCache->GetTagsData(hardcore))
        {
            FileReader reader(data->GetBuffer(), data->GetBufferSize());

            uint32_t count = reader.ReadInt32();
            reader.ReadInt32();     // This is the size of the string contents, but not really needed here

            for (size_t i = 0; i < count; ++i)
            {
                std::string tokenString = reader.ReadString();
                GameAPI::BestowTokenNow(player, tokenString);
            }
        }
    }
}

void ServerCoordinator::OnSetSeasonPlayerEvent(void* player, const SeasonInfo* seasonInfo)
{
    if (EngineAPI::IsMainCampaign())
    {
        LoadQuestStatesForPlayer(player);
        LoadSeasonTagsForPlayer(player);
    }

    spChat->Send("MutedList");
    spChat->Send("Welcome");
    if (uint32_t channel = ChatAPI::GetChatChannel())
        spChat->Send("JoinChannel", channel);

    if (uint32_t participantID = spCache->GetParticipantID(GameAPI::IsPlayerHardcore(player)))
    {
        spServer->Send("GetNewTradeNotificationCount", participantID);
        spServer->Send("GetParticipantPoints", participantID);
        spServer->Send("GetParticipantChallenges", seasonInfo->_seasonID, participantID);
    }
}

bool ServerCoordinator::OnCaravanInteractEvent(uint32_t caravanID)
{
    if (uint32_t participantID = spCache->GetParticipantID(EngineAPI::IsHardcore()))
    {
        spServer->Send("GetParticipantTransferQueue", participantID, caravanID);
        return false;
    }
    return true;
}

void ServerCoordinator::OnTransferPreSaveEvent()
{
    uint32_t participantID = spCache->GetParticipantID(EngineAPI::IsHardcore());
    const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();
    if (transferTabs.size() >= 6)
    {
        const std::map<uint32_t, EngineAPI::Rect>& items = GameAPI::GetItemsInTab(transferTabs[5]);
        if (items.size() > 0)
        {
            int32_t capacity = spCache->GetStashCapacity();
            if ((items.size() > capacity) && (capacity >= 0))
            {
                GameAPI::DisplayUINotification("tagGDLeagueStorageFull");
                return;
            }

            std::vector<uint32_t> storedItems;
            for (const auto& pair : items)
            {
                storedItems.emplace_back(pair.first);
                if (storedItems.size() >= 5)
                {
                    spServer->Send("StoreParticipantStashItems", participantID, storedItems);
                    storedItems.clear();
                }
            }

            if (storedItems.size() > 0)
                spServer->Send("StoreParticipantStashItems", participantID, storedItems);
        }
    }
}

void ServerCoordinator::OnDeleteFileEvent(const char* filename)
{
    std::filesystem::path filePath(filename);
    if (filePath.filename() == "player.gdc")
    {
        std::filesystem::path parentPath = filePath.parent_path();

        // Check to make sure it's a season character and not a custom game character before deleting
        if (parentPath.parent_path().filename() == "main")
        {
            Character characterData;
            if (!characterData.ReadFromFile(filename, true))
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
                return;
            }

            std::wstring characterName = characterData._headerBlock._charName;
            uint32_t participantID = spCache->GetParticipantID(characterData._headerBlock._charIsHardcore);

            spServer->Send("DeleteParticipantCharacter", participantID, characterName);
        }
    }
}

void ServerCoordinator::OnBestowTokenEvent(std::string token)
{
    if ((token.starts_with("gdl_")) && (spClient->IsPlayingSeason()))
    {
        uint32_t participantID = spCache->GetParticipantID(EngineAPI::IsHardcore());
        spServer->Send("SaveParticipantTag", participantID, token, EngineAPI::GetPlayerLevel(), GameAPI::GetGameDifficulty());
    }
}