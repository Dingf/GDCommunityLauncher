#ifndef INC_GDCL_DLL_CHAT_CLIENT_H
#define INC_GDCL_DLL_CHAT_CLIENT_H

#include <unordered_set>
#include <string>
#include <memory>
#include "Connection.h"
#include "EngineAPI/UI/ChatWindow.h"

class ChatClient : public Connection
{
    public:
        ~ChatClient();

        static ChatClient& GetInstance();

        uint8_t GetChannel(EngineAPI::UI::ChatType type) const;

        const std::string& GetConnectionID() const { return _connectionID; }
        const std::unordered_set<std::wstring>& GetMutedList() const { return _mutedList; }

        void SetChannel(EngineAPI::UI::ChatType type, uint32_t channel);
        void SetChannel(uint32_t channel) { SetChannel(EngineAPI::UI::CHAT_TYPE_NORMAL, channel); }

        bool ProcessChatCommand(std::wstring& name, std::wstring& message, uint8_t& type, void* item = nullptr);
        void SendChatMessage(EngineAPI::UI::ChatType type, const std::wstring& name, const std::wstring& message, void* item = nullptr);
        void SetChannelAndSendMessage(EngineAPI::UI::ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item);

        bool MutePlayer(std::wstring playerName);
        bool UnmutePlayer(std::wstring playerName);
        bool IsPlayerMuted(std::wstring playerName);

    private:
        typedef bool (*CommandInfoFilter)();
        typedef bool (*CommandHandler)(std::wstring&, std::wstring&, uint32_t&, uint8_t&, void*);

        struct CommandInfo
        {
            CommandInfoFilter     _filter;
            std::wstring          _blurb;
            std::wstring          _detail;
        };

        ChatClient(URI endpoint);
        ChatClient(ChatClient&) = delete;
        void operator=(const ChatClient&) = delete;

        static void OnShutdownEvent();
        static void OnWorldPreLoadEvent(std::string mapName, bool modded);
        static void OnSetMainPlayerEvent(void* player);

        static void OnConnection(const signalr::value& value);
        static void OnReceiveMessage(const signalr::value& value);
        static void OnServerMessage(const signalr::value& value);
        static void OnWelcomeMessage(const signalr::value& value);
        static void OnJoinedChannel(const signalr::value& value);
        static void OnBanned(const signalr::value& value);

        void LoadConfig();
        void SaveConfig();

        void LoadMutedList();
        void DisplayNewTradeNotifications();

        uint8_t _channels;
        std::string _connectionID;
        std::unordered_set<std::wstring> _mutedList;
};

#endif//INC_GDCL_DLL_CHAT_CLIENT_H