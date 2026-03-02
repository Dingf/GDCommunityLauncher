#include <unordered_set>
#include "EventManager.h"
#include "GameHandler.h"

bool HandleKeyEvent(void* _this, EngineAPI::Input::KeyButtonEvent& event)
{
    typedef bool (__thiscall* KeyEventProto)(void*, EngineAPI::Input::KeyButtonEvent&);

    KeyEventProto callback = (KeyEventProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        // There are multiple DisplayWidgets that are registered to HandleKeyEvent; to prevent duplication, choose only one of them to handle custom key events
        uint32_t unk1 = *((uint32_t*)(_this)+2);
        if (unk1 == 0x02)
        {
            if (EventManager::Poll(GDCL_EVENT_KEY_BUTTON_EVENT, event))
                return true;
        }

        return callback(_this, event);
    }
    return false;
}