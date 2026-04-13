#include "EngineAPI.h"
#include "GameAPI/Difficulty.h"
#include "ContextManager.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ServerHandler.h"
#include "ItemReplicaInfo.h"
#include "Configuration.h"
#include "JSON.h"

// Write handlers
std::string HandleWriteAddParticipant(uint32_t requestID, bool& hardcore);
std::string HandleWriteGetChallenges(uint32_t requestID, uint32_t& participantID, uint32_t& seasonID);
std::string HandleWriteGetCharacters(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteGetCharacterData(uint32_t requestID, uint32_t& participantID, std::wstring& characterName);
std::string HandleWriteGetCharacterFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName);
std::string HandleWriteSaveCharacterFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, std::string& base64Data);
std::string HandleWriteGetSeasons(uint32_t requestID);
std::string HandleWriteGetSeasonChallenges(uint32_t requestID, uint32_t& seasonID);
std::string HandleWriteGetPoints(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteGetTradeNotifications(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteGetTagFile(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteSaveTagFile(uint32_t requestID, uint32_t& participantID, std::string& base64Data);
std::string HandleWriteGetStashFile(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteSaveStashFile(uint32_t requestID, uint32_t& participantID, std::string& base64Data);
std::string HandleWriteGetTransmuteFile(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteSaveTransmuteFile(uint32_t requestID, uint32_t& participantID, std::string& base64Data);
std::string HandleWriteGetFormulasFile(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteSaveFormulasFile(uint32_t requestID, uint32_t& participantID, std::string& base64Data);
std::string HandleWriteGetQuestFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty);
std::string HandleWriteSaveQuestFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty, std::string& base64Data);
std::string HandleWriteGetConversationFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty);
std::string HandleWriteSaveConversationFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty, std::string& base64Data);
std::string HandleWriteGetMapFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty);
std::string HandleWriteSaveMapFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty, std::string& base64Data);
std::string HandleWriteGetFOWFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty);
std::string HandleWriteSaveFOWFile(uint32_t requestID, uint32_t& participantID, std::wstring& characterName, GameAPI::Difficulty& difficulty, std::string& base64Data);
std::string HandleWriteStashCapacity(uint32_t requestID);
std::string HandleWriteTransferItems(uint32_t requestID, uint32_t& participantID, std::vector<uint32_t>& itemIDs);
std::string HandleWriteStoreItems(uint32_t requestID, uint32_t& participantID, std::vector<ItemReplicaInfo>& items);
std::string HandleWriteTransferQueue(uint32_t requestID, uint32_t& participantID);
std::string HandleWriteDeleteCharacter(uint32_t requestID, uint32_t& participantID, std::wstring& characterName);
std::string HandleWriteSaveTag(uint32_t requestID, uint32_t& participantID, std::string& tagName, uint32_t& level, GameAPI::Difficulty& difficulty);

// Read Handlers
void HandleReadAddParticipant(const json& response, bool hardcore);
void HandleReadGetChallenges(const json& response, uint32_t participantID, uint32_t seasonID);
void HandleReadGetCharacters(const json& response, uint32_t participantID);
void HandleReadGetCharacterData(const json& response, uint32_t participantID, std::wstring characterName);
void HandleReadGetCharacterFile(const json& response, uint32_t participantID, std::wstring characterName);
void HandleReadSaveCharacterFile(const json& response, uint32_t participantID, std::wstring characterName, std::string base64Data);
void HandleReadGetSeasons(const json& response);
void HandleReadGetSeasonChallenges(const json& response, uint32_t seasonID);
void HandleReadGetPoints(const json& response, uint32_t participantID);
void HandleReadGetTradeNotifications(const json& response, uint32_t participantID);
void HandleReadGetTagFile(const json& response, uint32_t participantID);
void HandleReadSaveTagFile(const json& response, uint32_t participantID, std::string base64Data);
void HandleReadGetStashFile(const json& response, uint32_t participantID);
void HandleReadSaveStashFile(const json& response, uint32_t participantID, std::string base64Data);
void HandleReadGetTransmuteFile(const json& response, uint32_t participantID);
void HandleReadSaveTransmuteFile(const json& response, uint32_t participantID, std::string base64Data);
void HandleReadGetFormulasFile(const json& response, uint32_t participantID);
void HandleReadSaveFormulasFile(const json& response, uint32_t participantID, std::string base64Data);
void HandleReadGetQuestFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveQuestFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetConversationFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveConversationFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetMapFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveMapFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetFOWFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveFOWFile(const json& response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadStashCapacity(const json& response);
void HandleReadTransferItems(const json& response, uint32_t participantID, std::vector<uint32_t> itemIDs);
void HandleReadStoreItems(const json& response, uint32_t participantID, std::vector<ItemReplicaInfo> items);
void HandleReadTransferQueue(const json& response, uint32_t participantID);
void HandleReadDeleteCharacter(const json& response, uint32_t participantID, std::wstring characterName);
void HandleReadSaveTag(const json& response, uint32_t participantID, std::string tagName, uint32_t level, GameAPI::Difficulty difficulty);

const std::unordered_map<std::string, ServerHandler::HandlerPair> ServerHandler::_handlers =
{
    { "AddParticipant",                            { HandleWriteAddParticipant,        HandleReadAddParticipant } },
    { "GetParticipantChallenges",                  { HandleWriteGetChallenges,         HandleReadGetChallenges } },
    { "GetParticipantCharacters",                  { HandleWriteGetCharacters,         HandleReadGetCharacters } },
    { "GetCharacterData",                          { HandleWriteGetCharacterData,      HandleReadGetCharacterData } },
    { "GetCharacterFile",                          { HandleWriteGetCharacterFile,      HandleReadGetCharacterFile } },
    { "SaveCharacterFile",                         { HandleWriteSaveCharacterFile,     HandleReadSaveCharacterFile } },
    { "GetSeasons",                                { HandleWriteGetSeasons,            HandleReadGetSeasons } },
    { "GetSeasonChallenges",                       { HandleWriteGetSeasonChallenges,   HandleReadGetSeasonChallenges } },
    { "GetParticipantPoints",                      { HandleWriteGetPoints,             HandleReadGetPoints } },
    { "GetNewTradeNotificationCount",              { HandleWriteGetTradeNotifications, HandleReadGetTradeNotifications } },
    { "GetParticipantTagFile",                     { HandleWriteGetTagFile,            HandleReadGetTagFile } },
    { "SaveParticipantTagFile",                    { HandleWriteSaveTagFile,           HandleReadSaveTagFile } },
    { "GetParticipantStashFile",                   { HandleWriteGetStashFile,          HandleReadGetStashFile } },
    { "SaveParticipantStashFile",                  { HandleWriteSaveStashFile,         HandleReadSaveStashFile } },
    { "GetParticipantTransmutes",                  { HandleWriteGetTransmuteFile,      HandleReadGetTransmuteFile } },
    { "SaveParticipantTransmutes",                 { HandleWriteSaveTransmuteFile,     HandleReadSaveTransmuteFile } },
    { "GetParticipantFormulas",                    { HandleWriteGetFormulasFile,       HandleReadGetFormulasFile } },
    { "SaveParticipantFormulas",                   { HandleWriteSaveFormulasFile,      HandleReadSaveFormulasFile } },
    { "GetParticipantCharacterQuestFile",          { HandleWriteGetQuestFile,          HandleReadGetQuestFile } },
    { "SaveParticipantCharacterQuestFile",         { HandleWriteSaveQuestFile,         HandleReadSaveQuestFile } },
    { "GetParticipantCharacterConversationsFile",  { HandleWriteGetConversationFile,   HandleReadGetConversationFile } },
    { "SaveParticipantCharacterConversationsFile", { HandleWriteSaveConversationFile,  HandleReadSaveConversationFile } },
    { "GetParticipantCharacterMapDatFile",         { HandleWriteGetMapFile,            HandleReadGetMapFile } },
    { "SaveParticipantCharacterMapDatFile",        { HandleWriteSaveMapFile,           HandleReadSaveMapFile } },
    { "GetParticipantCharacterMapFowFile",         { HandleWriteGetFOWFile,            HandleReadGetFOWFile } },
    { "SaveParticipantCharacterMapFowFile",        { HandleWriteSaveFOWFile,           HandleReadSaveFOWFile } },
    { "GetParticipantSharedStashCapacity",         { HandleWriteStashCapacity,         HandleReadStashCapacity } },
    { "TransferParticipantItems",                  { HandleWriteTransferItems,         HandleReadTransferItems } },
    { "StoreParticipantStashItems",                { HandleWriteStoreItems,            HandleReadStoreItems } },
    { "GetParticipantTransferQueue",               { HandleWriteTransferQueue,         HandleReadTransferQueue } },
    { "DeleteParticipantCharacter",                { HandleWriteDeleteCharacter,       HandleReadDeleteCharacter } },
    { "SaveParticipantTag",                        { HandleWriteSaveTag,               HandleReadSaveTag }},
};

ServerHandler::ServerHandler()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,    &OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_POST_SHUTDOWN, &OnPostShutdownEvent);
    }
}

ServerHandler::~ServerHandler()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Unsubscribe(GDCL_EVENT_INITIALIZE,    &OnInitializeEvent);
        EventManager::Unsubscribe(GDCL_EVENT_POST_SHUTDOWN, &OnPostShutdownEvent);
    }
}

ServerHandler& ServerHandler::GetInstance()
{
    static ServerHandler instance;
    return instance;
}

Websocket<ServerHandler, std::future<json>>* ServerHandler::GetSocket()
{
    static Websocket<ServerHandler, std::future<json>> socket(ContextManager::GetIOContext(), ContextManager::GetSSLContext(), GetInstance());
    return &socket;
}

uint32_t ServerHandler::GetThreadCount()
{
    uint32_t numServerThreads = DEFAULT_SERVER_THREADS;

    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        const Value* serverThreadsValue = config.GetValue("Game", "server_threads");

        if ((serverThreadsValue) && (serverThreadsValue->GetType() == VALUE_TYPE_INT))
        {
            int32_t configThreads = serverThreadsValue->ToInt();
            if ((configThreads > 0) && (configThreads <= 16))
                numServerThreads = configThreads;
        }

        config.SetValue("Game", "server_threads", (int32_t)numServerThreads);
        config.Save(configPath);
    }
    return numServerThreads;
}

void ServerHandler::OnInitializeEvent()
{
    // Connect the client at game start; we can't do this when the DLL is loaded due to networking code
    ContextManager::Run();

    GetInstance().CreateThreadPool();

    uint32_t port = 443;
    std::string host = "gdcl-websocket.azurewebsites.net";

    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);

        const Value* hostnameValue = config.GetValue("Game", "hostname");
        if ((hostnameValue) && (hostnameValue->GetType() == VALUE_TYPE_STRING))
            host = hostnameValue->ToString();

        const Value* portValue = config.GetValue("Game", "port");
        if ((portValue) && (portValue->GetType() == VALUE_TYPE_INT))
            port = portValue->ToInt();

        config.SetValue("Game", "hostname", host);
        config.SetValue("Game", "port", (int32_t)port);
        config.Save(configPath);
    }

    spServer->Connect(host, port, "/account/connect", spClient->GetAuthToken());
}

void ServerHandler::OnPostShutdownEvent()
{
    spServer->Shutdown();
    ContextManager::Stop();
}