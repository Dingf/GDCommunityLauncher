#ifndef INC_GDCL_DLL_CHAT_API_H
#define INC_GDCL_DLL_CHAT_API_H

#include <string>
#include <unordered_set>

enum ChatType : uint8_t
{
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_SYSTEM = 1,
    CHAT_TYPE_GLOBAL = 2,
};

class ChatAPI
{
    public:
        static constexpr uint32_t CHAT_CHANNEL_MAX = 255;
        static constexpr uint32_t MAX_MESSAGE_SIZE = 255;

        ChatAPI(ChatAPI&) = delete;
        void operator=(const ChatAPI&) = delete;

        static ChatAPI* GetInstance();

        uint32_t GetChatColor(ChatType type);
        uint8_t  GetChatChannel();

        const std::wstring& GetChatPrefix();
        const std::wstring& GetBufferText();
        const std::wstring& GetSavedText();

        uint32_t GetCaratPosition();
        uint32_t GetSelectStartPosition();
        uint32_t GetSelectEndPosition();

        const std::unordered_set<std::wstring>& GetMutedList();

        bool IsPlayerMuted(std::wstring playerName);
        bool IsWindowVisible();
        bool HasChatWindow();

        bool SetChatColor(ChatType type, uint32_t color);
        void SetChatChannel(uint8_t channel);

        void SetCaratPosition(uint32_t position);
        void SetSelectStartPosition(uint32_t position);
        void SetSelectEndPosition(uint32_t position);

        void SetChatPrefix(const std::wstring& prefix);
        void SetBufferText(const std::wstring& text);
        void SetBufferTextDirect(const std::wstring& text);
        void SaveBufferText();

        bool MutePlayer(std::wstring playerName);
        bool UnmutePlayer(std::wstring playerName);
        void ClearMutedList();

        void SetServerMessagesEnabled(bool enabled);
        bool IsServerMessagesEnabled();
        bool IsGlobalChatEnabled();

        void ToggleWindowDisplay();

    private:
        ChatAPI();
        ~ChatAPI();

        void FindMagicAddresses();
        void LoadConfig();
        void SaveConfig();

        static void OnInitializeEvent();
        static void OnPreShutdownEvent();
        static void OnSetMainPlayerEvent(void* player);
        static void OnWorldPreUnloadEvent();

        bool _serverMessages;         // Whether or not server announcements are enabled
        uint8_t _channel;             // Current global chat channel
        uint8_t* _visibleAddress;     // Address used to toggle the chat window visibility
        uint8_t* _colorAddress;       // Address used to set the chat colors
        uint32_t _systemColor;        // Last used system color
        uint32_t _globalColor;        // Last used global color
        std::wstring _prefix;         // Last used chat prefix
        std::wstring _saved;          // Saved buffer text, used for linking items in chat
        std::unordered_set<std::wstring> _mutedList;    // List of muted players by the current user
};

#define spChat ChatAPI::GetInstance()

#endif//INC_GDCL_DLL_CHAT_API_H