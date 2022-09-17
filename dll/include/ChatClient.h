#ifndef INC_GDCL_DLL_CHAT_CLIENT_H
#define INC_GDCL_DLL_CHAT_CLIENT_H

#include <string>
#include <memory>
#include <signalrclient/hub_connection.h>
#include "EngineAPI/UI/ChatWindow.h"
#include "UpdateThread.h"

class ChatClient
{
    public:
        ~ChatClient();

        static ChatClient& GetInstance();

        uint8_t GetCurrentChatChannel(EngineAPI::UI::ChatType type) const;

        void SetCurrentChatChannel(EngineAPI::UI::ChatType type, uint32_t channel);
        void SetCurrentChatChannel(uint32_t channel);

        void SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message);

        void SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message);

        void DisplayWelcomeMessage();

    private:
        ChatClient();

        static void UpdateChatConnection();

        static void OnConnectEvent(void* data);
        static void OnDisconnectEvent(void* data);

        static void OnConnection(const signalr::value& m);
        static void OnReceiveMessage(const signalr::value& m);
        static void OnServerMessage(const signalr::value& m);
        static void OnWelcomeMessage(const signalr::value& m);
        static void OnJoinedChannel(const signalr::value& m);
        static void OnBanned(const signalr::value& m);

        uint8_t _channels;
        std::string _connectionID;
        std::unique_ptr<signalr::hub_connection> _connection;
        std::shared_ptr<UpdateThread<>> _chatConnectionThread;

};

#endif//INC_GDCL_DLL_CHAT_CLIENT_H