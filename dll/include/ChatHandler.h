#ifndef INC_GDCL_CHAT_HANDLER_H
#define INC_GDCL_CHAT_HANDLER_H

#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include "EngineAPI/Input/KeyButtonEvent.h"
#include "CallbackHandler.h"
#include "Websocket.h"
#include "JSON.h"

class ChatHandler : public CallbackHandler
{
    public:
        static Websocket<ChatHandler, std::future<json>>* GetSocket();

    private:
        ChatHandler();
        ~ChatHandler();
        ChatHandler(ChatHandler&) = delete;
        void operator=(const ChatHandler&) = delete;

        friend bool InitializeModules();

        const std::unordered_map<std::string, HandlerPair>& GetHandlers() const { return _handlers; }
        uint32_t GetThreadCount();
        void CreateRepeatKeyThread();

        static ChatHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnPostShutdownEvent();
        static bool OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event);

        static const std::unordered_map<std::string, HandlerPair> _handlers;
        static constexpr uint32_t DEFAULT_CHAT_THREADS = 2;

        std::atomic_int64_t              _repeatTime;
        EngineAPI::Input::KeyButtonEvent _repeatEvent;
};

#define spChat ChatHandler::GetSocket()

#endif//INC_GDCL_CHAT_HANDLER_H