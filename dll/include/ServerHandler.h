#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include <atomic>
#include <functional>
#include <future>
#include <unordered_map>
#include "Websocket.h"
#include "JSON.h"
#include "Log.h"

class ServerHandler
{
    public:
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;

        static bool Initialize();

        static Websocket<ServerHandler, std::future<json>>* GetSocket();

        template <typename... Ts>
        std::future<json> OnWrite(std::string& message, const std::string& name, Ts... args)
        {
            typedef std::string (__thiscall* WriteHandlerProto)(uint32_t, Ts...);
            typedef void (__thiscall* ReadHandlerProto)(json, Ts...);

            auto it = _handlers.find(name);
            if (it != _handlers.end())
            {
                uint32_t requestID = _requestCount++;
                ReadHandlerProto read = (ReadHandlerProto)it->second._readHandler;

                // Store the bound read function callback so that we can call it later upon receiving a response from the server
                _callbacks[requestID] = [read, args...](json j) { read(j, args...); };
                _promises[requestID] = {};

                message = ((WriteHandlerProto)it->second._writeHandler)(requestID, args...);

                return _promises[requestID].get_future();
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "No handler found for \"%\".", name);
                return {};
            }
        }

        void OnRead(const std::string& data);

    private:
        typedef std::function<void(json)> ReadHandlerCallback;

        struct ServerHandlerPair
        {
            void* _writeHandler;
            void* _readHandler;
        };

        ServerHandler();

        static ServerHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnShutdownEvent();

        std::atomic_uint32_t _requestCount;      // Request counter used to assign each request a unique ID
        std::unordered_map<uint32_t, ReadHandlerCallback> _callbacks;
        std::unordered_map<uint32_t, std::promise<json>>  _promises;

        static const std::unordered_map<std::string, ServerHandlerPair> _handlers;
};

#define spServer ServerHandler::GetSocket()

#endif//INC_GDCL_DLL_SERVER_HANDLER_H