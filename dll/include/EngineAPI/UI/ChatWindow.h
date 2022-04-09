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

        void ToggleDisplay();
        bool GetState();

    private:
        ChatWindow() : _visible(nullptr) {}

        void FindVisibleBit();

        uint8_t* _visible;  // Bit used to toggle the chat window visibility

};

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H