#include "EngineAPI.h"
#include "GameAPI/Difficulty.h"
#include "ContextManager.h"
#include "EventManager.h"
#include "SeasonClient.h"
#include "ServerHandler.h"
#include "Item.h"

// Write handlers
std::string HandleWriteAddParticipant(uint32_t seasonID);
std::string HandleWriteGetChallenges(uint32_t participantID, uint32_t seasonID);
std::string HandleWriteGetCharacters(uint32_t participantID);
std::string HandleWriteGetCharacterData(uint32_t participantID, std::wstring characterName);
std::string HandleWriteGetCharacterFile(uint32_t participantID, std::wstring characterName);
std::string HandleWriteGetSeasons();
std::string HandleWriteGetPoints(uint32_t participantID);
std::string HandleWriteGetTradeNotifications(uint32_t participantID);
std::string HandleWriteGetTagFile(uint32_t participantID);
std::string HandleWriteSaveTagFile(uint32_t participantID, std::string base64Data);
std::string HandleWriteGetStashFile(uint32_t participantID);
std::string HandleWriteSaveStashFile(uint32_t participantID, std::string base64Data);
std::string HandleWriteGetTransmuteFile(uint32_t participantID);
std::string HandleWriteSaveTransmuteFile(uint32_t participantID, std::string base64Data);
std::string HandleWriteGetFormulasFile(uint32_t participantID);
std::string HandleWriteSaveFormulasFile(uint32_t participantID, std::string base64Data);
std::string HandleWriteGetQuestFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveQuestFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetConversationFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveConversationFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetMapFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveMapFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteGetFOWFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty);
std::string HandleWriteSaveFOWFile(uint32_t participantID, std::wstring characterName, GameAPI::Difficulty difficulty, std::string base64Data);
std::string HandleWriteStashCapacity();
std::string HandleWriteTransferItems(uint32_t participantID, std::vector<uint32_t> itemIDs);
std::string HandleWriteStoreItems(uint32_t participantID, std::vector<Item> items);
std::string HandleWriteTransferQueue(uint32_t participantID);
std::string HandleWriteDeleteCharacter(uint32_t participantID, std::wstring characterName);

// Read Handlers
void HandleReadAddParticipant(json response);
void HandleReadGetChallenges(json response);
void HandleReadGetCharacters(json response);
void HandleReadGetCharacterData(json response);
void HandleReadGetCharacterFile(json response);
void HandleReadGetSeasons(json response);
void HandleReadGetPoints(json response);
void HandleReadGetTradeNotifications(json response);
void HandleReadGetTagFile(json response);
void HandleReadSaveTagFile(json response);
void HandleReadGetStashFile(json response);
void HandleReadSaveStashFile(json response);
void HandleReadGetTransmuteFile(json response);
void HandleReadSaveTransmuteFile(json response);
void HandleReadGetFormulasFile(json response);
void HandleReadSaveFormulasFile(json response);
void HandleReadGetQuestFile(json response);
void HandleReadSaveQuestFile(json response);
void HandleReadGetConversationFile(json response);
void HandleReadSaveConversationFile(json response);
void HandleReadGetMapFile(json response);
void HandleReadSaveMapFile(json response);
void HandleReadGetFOWFile(json response);
void HandleReadSaveFOWFile(json response);
void HandleReadStashCapacity(json response);
void HandleReadTransferItems(json response);
void HandleReadStoreItems(json response);
void HandleReadTransferQueue(json response);
void HandleReadDeleteCharacter(json response);

const std::unordered_map<std::string, ServerHandler::ServerHandlerFunctions> ServerHandler::_handlers =
{
    { "AddParticipant",                            { HandleWriteAddParticipant,        HandleReadAddParticipant } },
    { "GetParticipantChallenges",                  { HandleWriteGetChallenges,         HandleReadGetChallenges } },
    { "GetParticipantCharacters",                  { HandleWriteGetCharacters,         HandleReadGetCharacters } },
    { "GetCharacterData",                          { HandleWriteGetCharacterData,      HandleReadGetCharacterData } },
    { "GetCharacterFile",                          { HandleWriteGetCharacterFile,      HandleReadGetCharacterFile } },
    { "GetSeasons",                                { HandleWriteGetSeasons,            HandleReadGetSeasons } },
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
        //EventManager::Subscribe(GDCL_EVENT_INITIALIZE,  &OnInitializeEvent);
        //EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,    &OnShutdownEvent);
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

Websocket<ServerHandler>* ServerHandler::GetSocket()
{
    static Websocket<ServerHandler> socket(ContextManager::GetIOContext(), ContextManager::GetSSLContext(), GetInstance());
    return &socket;
}

void ServerHandler::OnRead(const std::string& data)
{
    typedef void (__thiscall* ReadHandlerProto)(json);

    json response = json::parse(data);
    std::string name = response.at("ResponseName").get<std::string>();

    auto it = _handlers.find(name);
    if (it != _handlers.end())
    {
        return ((ReadHandlerProto)it->second._readFunction)(response);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "No read handler found for \"%\".", name);
    }
}

/*void ServerHandler::OnInitializeEvent()
{
    // Connect the client at game start; we can't do this when the DLL is loaded due to networking code
    spServerSocket->Connect();
}

void ServerHandler::OnShutdownEvent()
{
    // TODO: Upload cached buffers here
}*/