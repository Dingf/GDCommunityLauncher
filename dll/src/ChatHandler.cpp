#include <boost/asio/post.hpp>
#include "ChatAPI.h"
#include "EngineAPI.h"
#include "ChatHandler.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ContextManager.h"
#include "Configuration.h"

// Write handlers
std::string HandleWriteWelcome();
std::string HandleWriteOnline();
std::string HandleWriteJoinChannel(uint8_t channel);
std::string HandleWriteSendMessage(uint8_t channel, std::wstring message, std::wstring playerName, void* item);
std::string HandleWriteGetMutedList();
std::string HandleWriteMutePlayer(std::wstring playerName);
std::string HandleWriteUnmutePlayer(std::wstring playerName);

// Read handlers
void HandleReadWelcome(const json& response);
void HandleReadOnline(const json& response);
void HandleReadJoinChannel(const json& response);
void HandleReadSendMessage(const json& response);
void HandleReadGetMutedList(const json& response);
void HandleReadMutePlayer(const json& response);
void HandleReadUnmutePlayer(const json& response);

// Custom KeyEvent handler
bool HandleCustomKeyEvent(EngineAPI::Input::KeyButtonEvent& event);

const std::unordered_map<std::string, std::pair<void*, void*>>& ChatHandler::GetHandlers() const
{
    static const std::unordered_map<std::string, std::pair<void*, void*>> handlers =
    {
        { "Welcome",     { HandleWriteWelcome,      HandleReadWelcome }},
        { "Online",      { HandleWriteOnline,       HandleReadOnline }},
        { "JoinChannel", { HandleWriteJoinChannel,  HandleReadJoinChannel }},
        { "Send",        { HandleWriteSendMessage,  HandleReadSendMessage }},
        { "MutedList",   { HandleWriteGetMutedList, HandleReadGetMutedList }},
        { "Mute",        { HandleWriteMutePlayer,   HandleReadMutePlayer }},
        { "Unmute",      { HandleWriteUnmutePlayer, HandleReadUnmutePlayer }},
    };
    return handlers;
}

ChatHandler::ChatHandler()
{
    _repeatTime = 0;
    _repeatEvent._key = EngineAPI::Input::KEY_NONE;

    if (!spClient->IsOfflineMode())
    {
        EventManager::Subscribe(GDCL_EVENT_INITIALIZE,       OnInitializeEvent);
        EventManager::Subscribe(GDCL_EVENT_POST_SHUTDOWN,    OnPostShutdownEvent);
        EventManager::Subscribe(GDCL_EVENT_KEY_BUTTON_EVENT, OnKeyButtonEvent);
    }
}

ChatHandler::~ChatHandler()
{
    EventManager::Unsubscribe(GDCL_EVENT_INITIALIZE,       OnInitializeEvent);
    EventManager::Unsubscribe(GDCL_EVENT_POST_SHUTDOWN,    OnPostShutdownEvent);
    EventManager::Unsubscribe(GDCL_EVENT_KEY_BUTTON_EVENT, OnKeyButtonEvent);
}

ChatHandler& ChatHandler::GetInstance()
{
    static ChatHandler instance;
    return instance;
}

Websocket<ChatHandler, bool>* ChatHandler::GetSocket()
{
    static Websocket<ChatHandler, bool> socket(ContextManager::GetIOContext(), ContextManager::GetSSLContext(), GetInstance());
    return &socket;
}

void ChatHandler::OnRead(const std::string& data)
{
    if (_threadPool)
    {
        try
        {
            json response = json::parse(data);
            const json& requestName = response.at("RequestName");
            if (requestName.is_null())
            {
                throw std::runtime_error(response.at("ErrorMessage").get<std::string>());
            }
            else if (requestName.is_string())
            {
                const auto& handlers = GetHandlers();
                auto it = handlers.find(requestName.get<std::string>());
                if (it != handlers.end())
                {
                    typedef void (*ReadHandlerProto)(json);

                    ReadHandlerProto callback = (ReadHandlerProto)it->second.second;
                    boost::asio::post(*_threadPool, [callback, response]()
                    {
                        try
                        {
                            callback(response);
                        }
                        catch (const std::exception& ex)
                        {
                            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to handle chat websocket message: %", ex.what());
                        }
                    });
                }
            }
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to parse chat websocket message: %", ex.what());
        }
    }
}

void ChatHandler::OnShutdown()
{
    if (_threadPool)
        _threadPool->join();
}
void ChatHandler::CreateThreadPool()
{
    if (!_threadPool)
    {
        uint32_t numChatThreads = DEFAULT_CHAT_THREADS;

        Configuration config;
        std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
        if (std::filesystem::is_regular_file(configPath))
        {
            config.Load(configPath);
            const Value* chatThreadsValue = config.GetValue("Chat", "chat_threads");

            if ((chatThreadsValue) && (chatThreadsValue->GetType() == VALUE_TYPE_INT))
            {
                int32_t configThreads = chatThreadsValue->ToInt();
                if ((configThreads > 1) && (configThreads <= 16))   // Need to have at least two threads in the pool because one will be used by the repeat key handler
                    numChatThreads = configThreads;
            }

            config.SetValue("Chat", "chat_threads", (int32_t)numChatThreads);
            config.Save(configPath);
        }

        _threadPool = std::make_unique<boost::asio::thread_pool>(numChatThreads);
    }
}

void ChatHandler::OnInitializeEvent()
{
    GetInstance().CreateThreadPool();
    GetInstance().CreateRepeatKeyThread();

    uint32_t port = 443;
    std::string host = "gdcl-chat.azurewebsites.net";

    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);

        const Value* hostnameValue = config.GetValue("Chat", "hostname");
        if ((hostnameValue) && (hostnameValue->GetType() == VALUE_TYPE_STRING))
            host = hostnameValue->ToString();

        const Value* portValue = config.GetValue("Chat", "port");
        if ((portValue) && (portValue->GetType() == VALUE_TYPE_INT))
            port = portValue->ToInt();

        config.SetValue("Chat", "hostname", host);
        config.SetValue("Chat", "port", (int32_t)port);
        config.Save(configPath);
    }

    spChat->Connect(host, port, "/chat/connect", spClient->GetAuthToken());
}

void ChatHandler::OnPostShutdownEvent()
{
    GetInstance()._repeatTime = -1;
    GetInstance()._repeatTime.notify_one();

    spChat->Shutdown();
}

bool ChatHandler::OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event)
{
    if ((spClient->IsPlayingSeasonOrOffline()) && (!EngineAPI::IsMultiplayer()) && (ChatAPI::HasChatWindow()))
    {
        auto& handler = GetInstance();
        if (event._state == EngineAPI::Input::KEY_STATE_DOWN)
        {
            // This needs to be set here because the hold event also calls HandleCustomKeyEvent() to handle the repeated inputs
            handler._repeatEvent = event;
            handler._repeatTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + 500;
            handler._repeatTime.notify_one();
            return HandleCustomKeyEvent(event);
        }
        else
        {
            handler._repeatEvent._key = EngineAPI::Input::KEY_NONE;
            handler._repeatTime = 0;
            handler._repeatTime.notify_one();
        }
    }
    return false;
}

void ChatHandler::CreateRepeatKeyThread()
{
    boost::asio::post(*_threadPool, [this]()
    {
        while (true)
        {
            if (_repeatTime > 0)
            {
                int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                if ((now >= _repeatTime) && (_repeatEvent._key != EngineAPI::Input::KEY_NONE))
                    HandleCustomKeyEvent(_repeatEvent);

                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            }
            else if (_repeatTime == 0)
            {
                _repeatTime.wait(0);
            }
            else
            {
                break;
            }
        }
    });
}