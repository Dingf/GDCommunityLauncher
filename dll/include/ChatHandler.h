#ifndef INC_GDCL_CHAT_HANDLER_H
#define INC_GDCL_CHAT_HANDLER_H

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/asio/thread_pool.hpp>
#include "EngineAPI/Input/KeyButtonEvent.h"
#include "Websocket.h"
#include "JSON.h"

class ChatHandler
{
    public:
        static Websocket<ChatHandler, bool>* GetSocket();

        template <typename... Ts>
        bool OnWrite(std::string& message, const std::string& name, Ts... args)
        {
            typedef std::string (*WriteHandlerProto)(Ts...);

            const auto& handlers = GetHandlers();
            auto it = handlers.find(name);
            if (it != handlers.end())
            {
                message = ((WriteHandlerProto)it->second.first)(args...);
                return true;
            }
            else
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find chat handler for message \"%\".", name);
            }
            return false;
        }

        void OnRead(const std::string& data);
        void OnShutdown();

        uint32_t GetBufferSize() const { return CHAT_BUFFER_SIZE; }

    private:
        ChatHandler();
        ~ChatHandler();
        ChatHandler(ChatHandler&) = delete;
        void operator=(const ChatHandler&) = delete;

        friend bool InitializeModules();

        const std::unordered_map<std::string, std::pair<void*,void*>>& GetHandlers() const;
        void CreateThreadPool();
        void CreateRepeatKeyThread();

        static ChatHandler& GetInstance();

        static void OnInitializeEvent();
        static void OnPostShutdownEvent();
        static bool OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event);

        static constexpr uint32_t DEFAULT_CHAT_THREADS = 2;
        static constexpr uint32_t CHAT_BUFFER_SIZE = 65536;

        std::atomic_int64_t              _repeatTime;
        EngineAPI::Input::KeyButtonEvent _repeatEvent;
        std::unique_ptr<boost::asio::thread_pool> _threadPool;
};

#define spChat ChatHandler::GetSocket()

#endif//INC_GDCL_CHAT_HANDLER_H