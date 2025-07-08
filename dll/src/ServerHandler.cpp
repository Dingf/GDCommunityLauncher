#include "EventManager.h"
#include "SeasonClient.h"
#include "ServerHandler.h"
#include "Log.h"

ServerHandler::ServerHandler()
{
    if (!spClient->IsOfflineMode())
    {
        //EventManager::Subscribe(GDCL_EVENT_INITIALIZE,  &ServerSync::OnInitializeEvent);
        //EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,    &ServerSync::OnShutdownEvent);
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
    static Websocket<ServerHandler> socket(GetInstance());
    return &socket;
}

void ServerHandler::operator<<(const std::string&)
{
    // TODO: Implement me
}

void ServerHandler::operator>>(const std::string&)
{
    // TODO: Implement me
}