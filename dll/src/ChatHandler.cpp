#include <string>
#include "ChatHandler.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ContextManager.h"

// Write handlers
std::string HandleWriteWelcome(uint32_t requestID);
std::string HandleWriteOnline(uint32_t requestID);
std::string HandleWriteJoinChannel(uint32_t requestID, uint8_t channel);
std::string HandleWriteSendMessage(uint32_t requestID, uint8_t channel, std::wstring message, std::wstring playerName, void* item);
std::string HandleWriteGetMutedList(uint32_t requestID);
std::string HandleWriteMutePlayer(uint32_t requestID, std::wstring playerName);
std::string HandleWriteUnmutePlayer(uint32_t requestID, std::wstring playerName);

// Read handlers
void HandleReadWelcome(const json& response);
void HandleReadOnline(const json& response);
void HandleReadJoinChannel(const json& response, uint8_t channel);
void HandleReadSendMessage(const json& response, uint8_t channel, std::wstring message, std::wstring playerName, void* item);
void HandleReadGetMutedList(const json& response);
void HandleReadMutePlayer(const json& response, std::wstring playerName);
void HandleReadUnmutePlayer(const json& response, std::wstring playerName);

const std::unordered_map<std::string, ChatHandler::HandlerPair> ChatHandler::_handlers =
{
    { "Welcome",     { HandleWriteWelcome,      HandleReadWelcome }},
    { "Online",      { HandleWriteOnline,       HandleReadOnline }},
    { "JoinChannel", { HandleWriteJoinChannel,  HandleReadJoinChannel }},
    { "Send",        { HandleWriteSendMessage,  HandleReadSendMessage }},
    { "MutedList",   { HandleWriteGetMutedList, HandleReadGetMutedList }},
    { "Mute",        { HandleWriteMutePlayer,   HandleReadMutePlayer }},
    { "Unmute",      { HandleWriteUnmutePlayer, HandleReadUnmutePlayer }},
};

ChatHandler::ChatHandler()
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,  &OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,    &OnShutdownEvent);
    }
}

ChatHandler& ChatHandler::GetInstance()
{
    static ChatHandler instance;
    return instance;
}

Websocket<ChatHandler, std::future<json>>* ChatHandler::GetSocket()
{
    static Websocket<ChatHandler, std::future<json>> socket(ContextManager::GetIOContext(), ContextManager::GetSSLContext(), GetInstance());
    return &socket;
}

void ChatHandler::SetPromiseData(std::promise<json>& promise, const json& json)
{
    promise.set_value(json);
}

void ChatHandler::OnInitializeEvent()
{
    // TODO: Get the URL value from the client instead of hardcoding it here
    spChat->Connect("gdcl-chat.azurewebsites.net", "443", "/chat/connect", spClient->GetAuthToken());
}

void ChatHandler::OnShutdownEvent()
{
    spChat->Disconnect();
}