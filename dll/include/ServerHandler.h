#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include "ServerCache.h"
#include "Websocket.h"

class ServerHandler
{
    public:
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;

        static bool Initialize();

        static Websocket<ServerHandler>* GetSocket();

        void operator<<(const std::string&);    // Write handler
        void operator>>(const std::string&);    // Read handler

    private:
        ServerHandler();

        ServerHandler& GetInstance();
};

#define spServerSocket ServerHandler::GetSocket();

#endif//INC_GDCL_DLL_SERVER_HANDLER_H