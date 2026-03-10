#ifndef INC_GDCL_DLL_CHAT_MANAGER_H
#define INC_GDCL_DLL_CHAT_MANAGER_H

#include <atomic>
#include <unordered_set>
#include <string>
#include <memory>
#include <thread>
#include "EngineAPI/Input/KeyButtonEvent.h"

enum ChatType : uint8_t
{
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_TRADE = 1,
    CHAT_TYPE_GLOBAL = 2,
    CHAT_TYPE_WHISPER = 3,
};

class ChatManager
{
    public:
        static constexpr uint32_t CHAT_CHANNEL_MAX = 15;
        static constexpr uint32_t MAX_MESSAGE_SIZE = 255;

        ChatManager(ChatManager&) = delete;
        void operator=(const ChatManager&) = delete;

        static ChatManager* GetInstance();

        static bool Initialize();

        uint32_t GetChatColor(ChatType type) const;
        uint8_t  GetChatChannel(ChatType type) const;

        const std::wstring& GetChatPrefix() const { return _prefix; }
        const std::wstring& GetBufferText() const;
        const std::wstring& GetSavedText() const { return _saved; }

        const std::unordered_set<std::wstring>& GetMutedList() const { return _mutedList; }

        bool IsPlayerMuted(std::wstring playerName);
        bool IsWindowVisible() const { return (_visible) ? (*_visible != 0) : false; }

        bool SetChatColor(ChatType type, uint32_t color);
        void SetChatChannel(ChatType type, uint32_t channel);

        void SetChatPrefix(const std::wstring& prefix) { _prefix = prefix; }
        void SaveBufferText() { _saved = GetBufferText(); }

        void SendChatMessage(ChatType type, const std::wstring& name, const std::wstring& message, void* item = nullptr);
        void SetChannelAndSendMessage(ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item);

        bool MutePlayer(std::wstring playerName);
        bool UnmutePlayer(std::wstring playerName);

    private:
        static constexpr int32_t HOLD_THREAD_STOP = -1;

        ChatManager();
        ~ChatManager();

        static void OnShutdownEvent();
        //static void OnWorldPreLoadEvent(std::string mapName, bool modded);
        static void OnWorldPreUnloadEvent();
        static void OnSetMainPlayerEvent(void* player);
        static bool OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event);

        /*static void OnConnection(const signalr::value& value);
        static void OnReceiveMessage(const signalr::value& value);
        static void OnServerMessage(const signalr::value& value);
        static void OnWelcomeMessage(const signalr::value& value);
        static void OnJoinedChannel(const signalr::value& value);
        static void OnBanned(const signalr::value& value);*/

        void LoadConfig();
        void SaveConfig();

        uint32_t& GetCaratPosition();
        uint32_t& GetSelectStartPosition();
        uint32_t& GetSelectEndPosition();
        std::wstring& GetBufferText();

        void SetBufferText(const std::wstring& text);

        void FindMagicAddresses();
        void ToggleWindowDisplay();

        void LoadMutedList();
        void DisplayNewTradeNotifications();

        void HoldThreadLoop();

        bool HandleKeyPress(EngineAPI::Input::KeyButtonEvent& event);

        uint8_t  _channels;     // Current chat channels; lower 4 bits are for trade chat, higher 4 bits are for global chat
        uint32_t _tradeColor;   // Color used for trade chat
        uint32_t _globalColor;  // Color used for global chat
        uint8_t* _visible;      // Address used to toggle the chat window visibility
        uint8_t* _colors;       // Address used to set the chat colors
        std::wstring _prefix;   // Last used chat prefix
        std::wstring _saved;    // Saved buffer text, used for linking items in chat
        std::unordered_set<std::wstring> _mutedList;    // List of muted players by the current user

        std::atomic_int64_t              _holdTime;     // Timestamp after which the thread should repeat inputs, by default +500ms after the original event
        EngineAPI::Input::KeyButtonEvent _holdEvent;    // Last down key event recorded; used by the hold thread to repeat key inputs
        std::unique_ptr<std::thread>     _holdThread;   // Thread used to repeat key inputs from held down keys

};

#define spChatManager ChatManager::GetInstance()

#endif//INC_GDCL_DLL_CHAT_MANAGER_H