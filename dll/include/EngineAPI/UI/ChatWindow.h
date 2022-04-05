#ifndef INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H
#define INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H

#include <stdint.h>

namespace EngineAPI::UI
{

class ChatWindow
{
    public:
        static ChatWindow& GetInstance(bool init = false);

        void ToggleDisplay();
        bool GetState();

    private:
        ChatWindow() : _magic(nullptr) {}

        void FindMagicBit();

        uint8_t* _magic;    // Magic bit used to toggle the chat window visibility
};

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_CHAT_WINDOW_H