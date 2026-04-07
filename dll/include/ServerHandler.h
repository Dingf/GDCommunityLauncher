#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include <string>
#include <future>
#include <unordered_map>
#include "CallbackHandler.h"
#include "Websocket.h"
#include "JSON.h"

class ServerHandler : public CallbackHandler
{
    public:
        static Websocket<ServerHandler, std::future<json>>* GetSocket();

    private:
        ServerHandler();
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;

        friend bool InitializeModules();

        const std::unordered_map<std::string, HandlerPair>& GetHandlers() const { return _handlers; }
        uint32_t GetThreadCount();

        static ServerHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnShutdownEvent();

        static const std::unordered_map<std::string, HandlerPair> _handlers;
        static constexpr uint32_t DEFAULT_SERVER_THREADS = 8;

};

#define spServer ServerHandler::GetSocket()

#endif//INC_GDCL_DLL_SERVER_HANDLER_H