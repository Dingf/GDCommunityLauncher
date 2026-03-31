#include <string>
#include "ChatAPI.h"
#include "EngineAPI.h"
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

// Custom KeyEvent handler
bool HandleCustomKeyEvent(EngineAPI::Input::KeyButtonEvent& event);

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

ChatHandler::ChatHandler(uint32_t threadCount) : CallbackHandler(threadCount)
{
    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,       &OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,         &OnShutdownEvent);
        EventManager::Subscribe(GDCL_EVENT_KEY_BUTTON_EVENT, &OnKeyButtonEvent);
    }
}

ChatHandler::~ChatHandler()
{
    EventManager::Unsubscribe(GDCL_EVENT_INITIALIZE,       &OnInitializeEvent);
    EventManager::Unsubscribe(GDCL_EVENT_SHUTDOWN,         &OnShutdownEvent);
    EventManager::Unsubscribe(GDCL_EVENT_KEY_BUTTON_EVENT, &OnKeyButtonEvent);
}

ChatHandler& ChatHandler::GetInstance(uint32_t threadCount)
{
    static ChatHandler instance(threadCount);
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

bool ChatHandler::OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event)
{
    if ((spClient->IsPlayingSeasonOrOffline()) && (!EngineAPI::IsMultiplayer()) && (ChatAPI::HasChatWindow()))
    {
        auto& thread = GetInstance()._repeatThread;
        if (event._state == EngineAPI::Input::KEY_STATE_DOWN)
        {
            // This needs to be set here because the hold event also calls HandleCustomKeyEvent() to handle the repeated inputs
            thread._repeatEvent = event;
            thread._repeatTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + 500;
            return HandleCustomKeyEvent(event);
        }
        else
        {
            thread._repeatEvent._key = EngineAPI::Input::KEY_NONE;
            thread._repeatTime = 0;
        }
        thread._repeatTime.notify_one();
    }
    return false;
}

ChatHandler::RepeatKeyThread::RepeatKeyThread()
{
    _running = true;
    _repeatTime = 0;
    _repeatEvent._key = EngineAPI::Input::KEY_NONE;
    _thread = std::make_unique<std::thread>([this](){ (*this)(); });
}

ChatHandler::RepeatKeyThread::~RepeatKeyThread()
{
    // Break the loop and wake up the thread if it's currently sleeping so we can join it
    _running = false;
    _repeatTime = 1;
    _repeatTime.notify_one();

    _thread->join();
    _thread.reset();
}

void ChatHandler::RepeatKeyThread::operator()()
{
    while (_running)
    {
        if (_repeatTime > 0)
        {
            int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if ((now >= _repeatTime) && (_repeatEvent._key != EngineAPI::Input::KEY_NONE))
                HandleCustomKeyEvent(_repeatEvent);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else
        {
            _repeatTime.wait(0);
        }
    }
}

