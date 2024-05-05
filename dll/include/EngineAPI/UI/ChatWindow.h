#ifndef INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H
#define INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H

#include <stdint.h>
#include "EngineAPI/Input/KeyButtonEvent.h"

namespace EngineAPI::UI
{

enum ChatType
{
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_TRADE = 1,
    CHAT_TYPE_GLOBAL = 2,
    CHAT_TYPE_WHISPER = 3,
};

static const uint32_t CHAT_CHANNEL_MAX = 15;
static const uint32_t MAX_CHAT_SIZE = 255;

class ChatWindow
{
    public:
        static ChatWindow& GetInstance(bool init = false);

        void SetChatPrefix(const std::wstring& prefix) { _prefix = prefix; }

        uint32_t GetChatColor(ChatType type) const;
        bool SetChatColor(ChatType type, uint32_t color, bool save = true);

        bool HandleKeyEvent(EngineAPI::Input::KeyButtonEvent& event);

        const std::wstring& GetBufferText() const { return *(std::wstring*)(_visible + 0xB0); }
        const std::wstring& GetSavedText() const { return _saved; }
        void SaveBufferText() { _saved = GetBufferText(); }

        bool IsInitialized() const { return ((_visible != nullptr) && (_colors != nullptr)); }
        bool IsVisible() const;

    private:
        ChatWindow() : _visible(nullptr), _colors(nullptr) {}
        ChatWindow(ChatWindow&) = delete;
        void operator=(const ChatWindow&) = delete;

        uint32_t& GetCaratPosition() const { return *(uint32_t*)(_visible + 0x160); }
        uint32_t& GetSelectStartPosition() const { return *(uint32_t*)(_visible + 0x164); }
        uint32_t& GetSelectEndPosition() const { return *(uint32_t*)(_visible + 0x168); }

        static void HoldKeyUpdate();

        bool HandleKeyPress(EngineAPI::Input::KeyButtonEvent& event);

        void SetCaratPosition(uint32_t position);
        void SetSelectStartPosition(uint32_t position);
        void SetSelectEndPosition(uint32_t position);
        void SetBufferText(const std::wstring& text);

        void ToggleDisplay();

        void FindMagicAddresses();
        void LoadConfig();
        void SaveConfig();

        uint8_t* _visible;      // Address used to toggle the chat window visibility
        uint8_t* _colors;       // Address used to set the chat colors
        std::wstring _prefix;   // Last used chat prefix
        std::wstring _saved;    // Saved buffer text, used for linking items in chat
        uint32_t _globalColor;  // Color used for global chat
        uint32_t _tradeColor;   // Color used for trade chat

        bool _holdLock;         // Lock used to prevent the hold thread from interfering with regular input
        EngineAPI::Input::KeyButtonEvent _holdEvent;    // Last key event that was held down, used for repeated inputs
};

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H