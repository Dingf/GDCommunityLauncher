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
        ServerHandler(uint32_t threadCount);
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;

        friend bool InitializeModules();

        const std::unordered_map<std::string, HandlerPair>& GetHandlers() const { return _handlers; }
        void SetPromiseData(std::promise<json>& promise, const json& json);

        static ServerHandler& GetInstance(uint32_t threadCount = 1);

        static void OnInitializeEvent();
        static void OnShutdownEvent();

        static const std::unordered_map<std::string, HandlerPair> _handlers;
};

#define spServer ServerHandler::GetSocket()

#endif//INC_GDCL_DLL_SERVER_HANDLER_H