#ifndef INC_GDCL_CHAT_HANDLER_H
#define INC_GDCL_CHAT_HANDLER_H

#include <string>
#include <future>
#include <unordered_map>
#include "CallbackHandler.h"
#include "Websocket.h"
#include "JSON.h"

class ChatHandler : public CallbackHandler
{
    public:
        static Websocket<ChatHandler, std::future<json>>* GetSocket();

    private:
        ChatHandler();
        ChatHandler(ChatHandler&) = delete;
        void operator=(const ChatHandler&) = delete;

        friend bool InitializeModules();

        const std::unordered_map<std::string, HandlerPair>& GetHandlers() const { return _handlers; }
        void SetPromiseData(std::promise<json>& promise, const json& json);

        static ChatHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnShutdownEvent();

        static const std::unordered_map<std::string, HandlerPair> _handlers;
};

#define spChat ChatHandler::GetSocket()

#endif//INC_GDCL_CHAT_HANDLER_H