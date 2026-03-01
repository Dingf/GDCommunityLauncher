#include "EngineAPI.h"
#include "GameAPI/Difficulty.h"
#include "ContextManager.h"
#include "EventManager.h"
#include "SeasonClient.h"
#include "ServerHandler.h"
#include "Item.h"
#include "JSON.h"

// Write handlers
std::string HandleWriteAddParticipant(uint32_t requestID, uint32_t seasonID);
std::string HandleWriteGetChallenges(uint32_t requestID, uint32_t participantID, uint32_t seasonID);
std::string HandleWriteGetCharacters(uint32_t requestID, uint32_t participantID);
std::string HandleWriteGetCharacterData(uint32_t requestID, uint32_t participantID, std::wstring characterName);
std::string HandleWriteGetCharacterFile(uint32_t requestID, uint32_t participantID, std::wstring characterName);
std::string HandleWriteGetSeasons(uint32_t requestID);
std::string HandleWriteGetSeasonChallenges(uint32_t requestID, uint32_t seasonID);
std::string HandleWriteGetPoints(uint32_t requestID, uint32_t participantID);
std::string HandleWriteGetTradeNotifications(uint32_t requestID, uint32_t participantID);
std::string HandleWriteGetTagFile(uint32_t requestID, uint32_t participantID);
std::string HandleWriteSaveTagFile(uint32_t requestID, uint32_t participantID, std::string base64Data);
std::string HandleWriteGetStashFile(uint32_t requestID, uint32_t participantID);
std::string HandleWriteSaveStashFile(uint32_t requestID, uint32_t participantID, std::string base64Data);
std::string HandleWriteGetTransmuteFile(uint32_t requestID, uint32_t participantID);
std::string HandleWriteSaveTransmuteFile(uint32_t requestID, uint32_t participantID, std::string base64Data);
std::string HandleWriteGetFormulasFile(uint32_t requestID, uint32_t participantID);
std::string HandleWriteSaveFormulasFile(uint32_t requestID, uint32_t participantID, std::string base64Data);
std::string HandleWriteGetQuestFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveQuestFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetConversationFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveConversationFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetMapFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveMapFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetFOWFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveFOWFile(uint32_t requestID, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteStashCapacity(uint32_t requestID);
std::string HandleWriteTransferItems(uint32_t requestID, uint32_t participantID, std::vector<uint32_t> itemIDs);
std::string HandleWriteStoreItems(uint32_t requestID, uint32_t participantID, std::vector<Item> items);
std::string HandleWriteTransferQueue(uint32_t requestID, uint32_t participantID);
std::string HandleWriteDeleteCharacter(uint32_t requestID, uint32_t participantID, std::wstring characterName);

// Read Handlers
void HandleReadAddParticipant(json response, uint32_t seasonID);
void HandleReadGetChallenges(json response, uint32_t participantID, uint32_t seasonID);
void HandleReadGetCharacters(json response, uint32_t participantID);
void HandleReadGetCharacterData(json response, uint32_t participantID, std::wstring characterName);
void HandleReadGetCharacterFile(json response, uint32_t participantID, std::wstring characterName);
void HandleReadGetSeasons(json response);
void HandleReadGetSeasonChallenges(json response, uint32_t seasonID);
void HandleReadGetPoints(json response, uint32_t participantID);
void HandleReadGetTradeNotifications(json response, uint32_t participantID);
void HandleReadGetTagFile(json response, uint32_t participantID);
void HandleReadSaveTagFile(json response, uint32_t participantID, std::string base64Data);
void HandleReadGetStashFile(json response, uint32_t participantID);
void HandleReadSaveStashFile(json response, uint32_t participantID, std::string base64Data);
void HandleReadGetTransmuteFile(json response, uint32_t participantID);
void HandleReadSaveTransmuteFile(json response, uint32_t participantID, std::string base64Data);
void HandleReadGetFormulasFile(json response, uint32_t participantID);
void HandleReadSaveFormulasFile(json response, uint32_t participantID, std::string base64Data);
void HandleReadGetQuestFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveQuestFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetConversationFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveConversationFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetMapFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveMapFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadGetFOWFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
void HandleReadSaveFOWFile(json response, uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
void HandleReadStashCapacity(json response);
void HandleReadTransferItems(json response, uint32_t participantID, std::vector<uint32_t> itemIDs);
void HandleReadStoreItems(json response, uint32_t participantID, std::vector<Item> items);
void HandleReadTransferQueue(json response, uint32_t participantID);
void HandleReadDeleteCharacter(json response, uint32_t participantID, std::wstring characterName);

const std::unordered_map<std::string, ServerHandler::ServerHandlerPair> ServerHandler::_handlers =
{
    { "AddParticipant",                            { HandleWriteAddParticipant,        HandleReadAddParticipant } },
    { "GetParticipantChallenges",                  { HandleWriteGetChallenges,         HandleReadGetChallenges } },
    { "GetParticipantCharacters",                  { HandleWriteGetCharacters,         HandleReadGetCharacters } },
    { "GetCharacterData",                          { HandleWriteGetCharacterData,      HandleReadGetCharacterData } },
    { "GetCharacterFile",                          { HandleWriteGetCharacterFile,      HandleReadGetCharacterFile } },
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
};

ServerHandler::ServerHandler()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,  &OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,    &OnShutdownEvent);
    }
}

ServerHandler& ServerHandler::GetInstance()
{
    static ServerHandler instance;
    return instance;
}

bool ServerHandler::Initialize()
{
    try
    {
        GetInstance();
        return true;
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ServerHandler module: %", ex.what());
        return false;
    }
}

Websocket<ServerHandler, std::future<json>>* ServerHandler::GetSocket()
{
    static Websocket<ServerHandler, std::future<json>> socket(ContextManager::GetIOContext(), ContextManager::GetSSLContext(), GetInstance());
    return &socket;
}

void ServerHandler::OnRead(const std::string& data)
{
    try
    {
        json response = json::parse(data);
        uint32_t requestID = response.at("RequestId").get<uint32_t>();

        auto it = _callbacks.find(requestID);
        if (it != _callbacks.end())
        {
            it->second(response);
            _callbacks.erase(it);

            _promises[requestID].set_value(response.at("Data"));
            _promises.erase(requestID);
        }
        else
        {
            throw std::runtime_error("Could not find bound handler with requestID " + requestID);
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read server response: %\n%", ex.what(), data);
    }
}

void ServerHandler::OnInitializeEvent()
{
    // Connect the client at game start; we can't do this when the DLL is loaded due to networking code
    ContextManager::Run();

    // TODO: Get the URL value from the client instead of hardcoding it here
    spServer->Connect("gdcl-websocket.azurewebsites.net", "443", "/account/connect", spClient->GetAuthToken());
}

void ServerHandler::OnShutdownEvent()
{
    ContextManager::Stop();
    // TODO: Upload cached buffers here
}