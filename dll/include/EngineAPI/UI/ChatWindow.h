#ifndef INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H
#define INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H

#include <stdint.h>

namespace EngineAPI::UI
{

enum ChatType
{
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_TRADE = 1,
    CHAT_TYPE_GLOBAL = 2,
};

static const uint32_t CHAT_CHANNEL_MAX = 15;

class ChatWindow
{
    public:
        static ChatWindow& GetInstance(bool init = false);

        void SetChatPrefix(const std::wstring& prefix) { _prefix = prefix; }

        uint32_t GetChatColor(ChatType type);
        bool SetChatColor(ChatType type, uint32_t color, bool save = true);

        void ToggleDisplay();

        bool IsToggleInitialized() const { return (_visible != nullptr); }
        bool IsColorsInitialized() const { return (_colors != nullptr); }
        bool IsVisible();

    private:
        ChatWindow() : _visible(nullptr) {}

        void FindMagicAddresses();
        void LoadConfig();
        void SaveConfig();

        uint8_t* _visible;      // Address used to toggle the chat window visibility
        uint8_t* _colors;       // Address used to set the chat colors
        std::wstring _prefix;   // Last used chat prefix
        uint32_t _globalColor;  // Color used for global chat
        uint32_t _tradeColor;   // Color used for trade chat

};

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H