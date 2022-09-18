#include "ClientHandlers.h"

bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event)
{
    typedef bool(__thiscall* HandleKeyEventProto)(void*, EngineAPI::KeyButtonEvent&);

    HandleKeyEventProto callback = (HandleKeyEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();
        EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();

        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if (unk1 == 0x02)
        {
            // Enable the chat window while playing the league mod in single player
            if ((client.IsParticipatingInSeason()) && (event._keyCode == EngineAPI::KEY_ENTER) && (!EngineAPI::IsMultiplayer()) && (event._keyState == EngineAPI::KEY_STATE_DOWN))
            {
                if (!chatWindow.IsVisible())
                {
                    chatWindow.ToggleDisplay();
                    if (!chatWindow.IsVisible())
                    {
                        GameAPI::SendChatMessage(L"Server", L"Click here to open chat", EngineAPI::UI::CHAT_TYPE_NORMAL);
                    }
                }
            }
        }

        if ((client.IsParticipatingInSeason()) && (event._keyCode == EngineAPI::KEY_TILDE))
        {
            // Disable the tilde key to prevent console access
            return true;
        }
        else
        {
            return callback(_this, event);
        }
    }
    return false;
}