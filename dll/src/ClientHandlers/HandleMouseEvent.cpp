#include "ClientHandlers.h"
#include "ChatClient.h"
#include "ItemLinker.h"

bool HandleMouseEvent(void* _this, EngineAPI::MouseEvent& event)
{
    typedef bool(__thiscall* HandleMouseEventProto)(void*, EngineAPI::MouseEvent&);

    HandleMouseEventProto callback = (HandleMouseEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_MOUSE_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();

        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if (unk1 == 0x02)
        {
            // Link the item under the cursor when receiving Shift + Left Click while the chat window is open
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            if ((client.IsParticipatingInSeason()) && (event._action == EngineAPI::MOUSE_ACTION_LEFT) && (event._modifier == EngineAPI::MOUSE_MODIFIER_SHIFT) && (chatWindow.IsVisible()))
            {
                void* item = ItemLinker::GetLinkedItem();
                if (item)
                {
                    ChatClient& chatClient = ChatClient::GetInstance();
                    std::wstring name = client.GetActiveCharacterName();
                    std::wstring message = chatWindow.GetBufferText();
                    uint8_t type = EngineAPI::UI::CHAT_TYPE_NORMAL;

                    // If no interrupting chat command was processed, display the item link message in chat
                    if (chatClient.ProcessChatCommand(name, message, type, item))
                        GameAPI::AddChatMessage(name, message, type, item);

                    // Simulate pressing ESC to close out of the chat window properly
                    INPUT inputs[2] = {0};
                    inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
                    inputs[0].ki.dwFlags = inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE;
                    inputs[0].ki.wScan = inputs[1].ki.wScan = MapVirtualKey(VK_ESCAPE, 0);
                    inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
                    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

                    return true;
                }
            }
        }

        return callback(_this, event);
    }
    return false;
}