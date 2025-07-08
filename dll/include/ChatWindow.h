// TODO: Delete this file after cleaning up hold key on ChatManager
/*#ifndef INC_GDCL_DLL_CHAT_WINDOW_H
#define INC_GDCL_DLL_CHAT_WINDOW_H

#include <stdint.h>
#include "EngineAPI/Input/KeyButtonEvent.h"
#include "EngineAPI/UI/Chat.h"

class ChatWindow
{
    public:
        static ChatWindow* GetInstance();

        void SetChatPrefix(const std::wstring& prefix) { _prefix = prefix; }

        uint32_t GetChatColor(ChatType type) const;
        bool SetChatColor(ChatType type, uint32_t color);

        bool HandleKeyEvent(EngineAPI::Input::KeyButtonEvent& event);

        const std::wstring& GetBufferText() const { return *(std::wstring*)(_visible + 0xB0); }
        const std::wstring& GetSavedText() const { return _saved; }
        void SaveBufferText() { _saved = GetBufferText(); }

        bool IsVisible() const;

    private:
        ChatWindow();
        ChatWindow(ChatWindow&) = delete;
        void operator=(const ChatWindow&) = delete;

        static void OnSetMainPlayerEvent();
        static void OnWorldPreUnloadEvent();

        uint32_t& GetCaratPosition() const { return *(uint32_t*)(_visible + 0x160); }
        uint32_t& GetSelectStartPosition() const { return *(uint32_t*)(_visible + 0x164); }
        uint32_t& GetSelectEndPosition() const { return *(uint32_t*)(_visible + 0x168); }

        static int64_t HoldKeyUpdate();

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

#define spChatWindow EngineAPI::UI::ChatWindow::GetInstance()

#endif//INC_GDCL_DLL_CHAT_WINDOW_H*/