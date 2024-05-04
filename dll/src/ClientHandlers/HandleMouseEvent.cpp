#include "ClientHandler.h"
#include "ChatClient.h"

bool HandleMouseEvent(void* _this, EngineAPI::Input::MouseEvent& event)
{
    typedef bool (__thiscall* MouseEventProto)(void*, EngineAPI::Input::MouseEvent&);

    MouseEventProto callback = (MouseEventProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();

        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if ((unk1 == 0x02) && (client.IsPlayingSeason()))
        {
            // Save the buffer text when Ctrl + Shift + Left Click is received and the chat window is open
            // This allows us to retrieve the text when processing the chat command later
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            if ((event._action == EngineAPI::Input::MOUSE_ACTION_LEFT) && 
                ((event._modifier == EngineAPI::Input::KEY_MODIFIER_CTRL_SHIFT) || (event._modifier == EngineAPI::Input::KEY_MODIFIER_SHIFT)) && 
                (chatWindow.IsVisible()))
            {
                chatWindow.SaveBufferText();
            }
        }

        return callback(_this, event);
    }
    return false;
}