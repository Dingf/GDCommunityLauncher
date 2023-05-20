#ifndef INC_GDCL_DLL_CHAT_CLIENT_H
#define INC_GDCL_DLL_CHAT_CLIENT_H

#include <string>
#include <memory>
#include <signalrclient/hub_connection.h>
#include "EngineAPI/UI/ChatWindow.h"

class ChatClient
{
    public:
        ~ChatClient();

        static ChatClient& GetInstance();

        uint8_t GetChannel(EngineAPI::UI::ChatType type) const;

        const std::string& GetConnectionID() const { return _connectionID; }

        void SetChannel(EngineAPI::UI::ChatType type, uint32_t channel);
        void SetChannel(uint32_t channel);

        bool ProcessChatCommand(std::wstring& name, std::wstring& message, uint8_t& type, void* item = nullptr);
        void SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message, void* item = nullptr);
        void SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item = nullptr);

        void DisplayWelcomeMessage();
        void DisplayNewTradeNotifications();

    private:
        ChatClient();
        ChatClient(ChatClient&) = delete;
        void operator=(const ChatClient&) = delete;

        static void OnWorldLoadEvent(void* data);

        static void OnConnection(const signalr::value& m);
        static void OnReceiveMessage(const signalr::value& m);
        static void OnServerMessage(const signalr::value& m);
        static void OnWelcomeMessage(const signalr::value& m);
        static void OnJoinedChannel(const signalr::value& m);
        static void OnBanned(const signalr::value& m);

        void LoadConfig();
        void SaveConfig();

        uint8_t _channels;
        std::string _connectionID;
        std::unique_ptr<signalr::hub_connection> _connection;

};

#endif//INC_GDCL_DLL_CHAT_CLIENT_H