#include "ClientHandlers.h"
#include "ChatClient.h"

bool HandleMouseEvent(void* _this, EngineAPI::MouseEvent& event)
{
    typedef bool (__thiscall* MouseEventProto)(void*, EngineAPI::MouseEvent&);

    MouseEventProto callback = (MouseEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();

        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if ((unk1 == 0x02) && (client.IsParticipatingInSeason()))
        {
            // Save the buffer text when Ctrl + Shift + Left Click is received and the chat window is open
            // This allows us to retrieve the text when processing the chat command later
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            if ((event._action == EngineAPI::MOUSE_ACTION_LEFT) && ((event._modifier == EngineAPI::MOUSE_MODIFIER_SHIFT_CTRL) || (event._modifier == EngineAPI::MOUSE_MODIFIER_SHIFT)) && (chatWindow.IsVisible()))
            {
                chatWindow.SaveBufferText();
            }
        }

        return callback(_this, event);
    }
    return false;
}