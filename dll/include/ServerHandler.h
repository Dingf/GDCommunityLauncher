#ifndef INC_GDCL_DLL_SERVER_HANDLER_H
#define INC_GDCL_DLL_SERVER_HANDLER_H

#include <atomic>
#include <string>
#include <future>
#include <unordered_map>
#include <boost/asio/thread_pool.hpp>
#include "Websocket.h"
#include "JSON.h"

class ServerHandlerRequest
{
    public:
        friend class ServerHandler;

        ServerHandlerRequest() : _requestID(0), _callback(nullptr), _then(nullptr) { }
        ServerHandlerRequest(const ServerHandlerRequest& request)
        {
            _requestID = request._requestID;
            _callback = request._callback;
            _then = request._then;
            _promise = request._promise;
        }

        bool valid() const { return (_promise != nullptr); }
        json get() { return (_promise) ? _promise->get_future().get() : json(); }
        void wait() { if (_promise) _promise->get_future().wait(); }
        std::future<json> get_future() { return (_promise) ? _promise->get_future() : std::future<json>(); }

        ServerHandlerRequest& then(std::function<void(json)> then)
        {
            *_then = then;
            return *this;
        }

    private:
        ServerHandlerRequest(uint32_t requestID, std::function<void(json)> callback) : _requestID(requestID), _callback(callback)
        {
            _then = std::make_shared<std::function<void(const json&)>>(nullptr);
            _promise = std::make_shared<std::promise<json>>();
        }

        uint32_t _requestID;
        std::function<void(const json&)> _callback;
        std::shared_ptr<std::function<void(const json&)>> _then;
        std::shared_ptr<std::promise<json>> _promise;
};

class ServerHandler
{
    public:
        static Websocket<ServerHandler, ServerHandlerRequest>* GetSocket();

        template <typename... Ts>
        ServerHandlerRequest OnWrite(std::string& message, const std::string& name, Ts... args)
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
                _requests[requestID] = { requestID, [read, args...](const json& j) { read(j, args...); } };
                return _requests[requestID];
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find server handler for message \"%\".", name);
                return {};
            }
        }

        void OnRead(const std::string& data);
        void OnShutdown();

        uint32_t GetBufferSize() const { return SERVER_BUFFER_SIZE; }

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
        static constexpr uint32_t SERVER_BUFFER_SIZE = 262144;

        std::atomic_uint32_t _requestCount;      // Request counter used to assign each request a unique ID
        std::unordered_map<uint32_t, ServerHandlerRequest> _requests;
        std::unique_ptr<boost::asio::thread_pool> _threadPool;
};

#define spServer ServerHandler::GetSocket()

#endif//INC_GDCL_DLL_SERVER_HANDLER_H