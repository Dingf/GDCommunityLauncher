#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include <unordered_map>
#include "ServerCache.h"
#include "Websocket.h"
#include "Log.h"

class ServerHandler
{
    public:
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;

        static bool Initialize();

        static Websocket<ServerHandler>* GetSocket();

        template <typename... Ts>
        std::string OnWrite(const std::string& name, Ts... args)
        {
            typedef std::string (__thiscall* WriteHandlerProto)(Ts...);

            auto it = _handlers.find(name);
            if (it != _handlers.end())
            {
                return ((WriteHandlerProto)it->_writeFunction)(args...);
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "No write handler found for \"%\".", name);
            }
        }

        void OnRead(const std::string& data);

    private:
        struct ServerHandlerFunctions
        {
            void* _writeFunction;
            void* _readFunction;
        };

        ServerHandler();

        static ServerHandler& GetInstance();

        //void OnInitializeEvent();
        //void OnShutdownEvent();

        static const std::unordered_map<std::string, ServerHandlerFunctions> _handlers;
};

#define spServerSocket ServerHandler::GetSocket();

#endif//INC_GDCL_DLL_SERVER_HANDLER_H