#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include <atomic>
#include <string>
#include <future>
#include <unordered_map>
#include <boost/asio/thread_pool.hpp>
#include "Websocket.h"
#include "JSON.h"

class ServerHandler
{
    public:
        static Websocket<ServerHandler, std::future<json>>* GetSocket();

        template <typename... Ts>
        std::future<json> OnWrite(std::string& message, const std::string& name, Ts... args)
        {
            typedef std::string (*WriteHandlerProto)(uint32_t, Ts&...);
            typedef void (*ReadHandlerProto)(json, Ts...);

            const auto& handlers = GetHandlers();
            auto it = handlers.find(name);
            if (it != handlers.end())
            {
                uint32_t requestID = _requestCount++;
                ReadHandlerProto read = (ReadHandlerProto)it->second.second;

                message = ((WriteHandlerProto)it->second.first)(requestID, args...);

                // Store the bound read function callback so that we can call it later upon receiving a response from the server
                _callbacks[requestID] = [read, args...](json j) { read(j, args...); };
                _promises[requestID] = {};

                return _promises[requestID].get_future();
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find server handler for message \"%\".", name);
                return {};
            }
        }

        void OnRead(const std::string& data);
        void OnShutdown();

    private:
        ServerHandler();
        ServerHandler(ServerHandler&) = delete;
        void operator=(const ServerHandler&) = delete;
        ~ServerHandler();

        friend bool InitializeModules();

        const std::unordered_map<std::string, std::pair<void*,void*>>& GetHandlers() const;
        void CreateThreadPool();

        static ServerHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnPostShutdownEvent();

        static constexpr uint32_t DEFAULT_SERVER_THREADS = 8;

        std::atomic_uint32_t _requestCount;      // Request counter used to assign each request a unique ID
        std::unordered_map<uint32_t, std::function<void(json)>> _callbacks;
        std::unordered_map<uint32_t, std::promise<json>>  _promises;
        std::unique_ptr<boost::asio::thread_pool> _threadPool;
};

#define spServer ServerHandler::GetSocket()

#endif//INC_GDCL_DLL_SERVER_HANDLER_H