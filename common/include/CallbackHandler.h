#ifndef INC_GDCL_CALLBACK_HANDLER
#define INC_GDCL_CALLBACK_HANDLER

#include <string>
#include <future>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include "JSON.h"
#include "Log.h"

class CallbackHandler
{
    public:
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
                ReadHandlerProto read = (ReadHandlerProto)it->second._readHandler;

                message = ((WriteHandlerProto)it->second._writeHandler)(requestID, args...);

                // Store the bound read function callback so that we can call it later upon receiving a response from the server
                _callbacks[requestID] = [read, args...](json j) { read(j, args...); };
                _promises[requestID] = {};

                return _promises[requestID].get_future();
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find handler for message \"%\".", name);
                return {};
            }
        }

        void OnRead(const std::string& data)
        {
            boost::asio::post(_threadPool, [this, data]()
            {
                try
                {
                    json response = json::parse(data);
                    uint32_t requestID = response.at("RequestId").get<uint32_t>();

                    auto it = _callbacks.find(requestID);
                    if (it != _callbacks.end())
                    {
                        it->second(response);
                        _callbacks.erase(it);

                        SetPromiseData(_promises[requestID], response);
                        _promises.erase(requestID);
                    }
                    else
                    {
                        throw std::runtime_error("Could not find callback for request " + std::to_string(requestID));
                    }
                }
                catch (const std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read data from websocket: %", ex.what());
                }
            });
        }

    protected:
        typedef std::function<void(json)> ReadHandlerCallback;

        struct HandlerPair
        {
            void* _writeHandler;
            void* _readHandler;
        };

        CallbackHandler(uint32_t threadCount) : _threadPool(threadCount) {}

        virtual const std::unordered_map<std::string, HandlerPair>& GetHandlers() const = 0;
        virtual void SetPromiseData(std::promise<json>& promise, const json& json) = 0;

        std::atomic_uint32_t _requestCount;      // Request counter used to assign each request a unique ID
        std::unordered_map<uint32_t, ReadHandlerCallback> _callbacks;
        std::unordered_map<uint32_t, std::promise<json>>  _promises;
        boost::asio::thread_pool _threadPool;

};

#endif//INC_GDCL_CALLBACK_HANDLER