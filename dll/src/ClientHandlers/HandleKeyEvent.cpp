#include <unordered_set>
#include "ClientHandler.h"

bool HandleKeyEvent(void* _this, EngineAPI::Input::KeyButtonEvent& event)
{
    typedef bool (__thiscall* KeyEventProto)(void*, EngineAPI::Input::KeyButtonEvent&);

    KeyEventProto callback = (KeyEventProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();

        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if (unk1 == 0x02)
        {
            // Enable the chat window while playing the season in single player
            if ((client.IsPlayingSeason()) && (!EngineAPI::IsMultiplayer()))
            {
                EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
                if (chatWindow.IsInitialized())
                {
                    if (chatWindow.HandleKeyEvent(event))
                        return true;
                }
            }
        }

        return callback(_this, event);
    }
    return false;
}