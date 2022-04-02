#include "ClientHandlers.h"
#include "Log.h"

bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event)
{
    typedef bool(__thiscall* HandleKeyEventProto)(void*, EngineAPI::KeyButtonEvent&);

    HandleKeyEventProto callback = (HandleKeyEventProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_HANDLE_KEY_EVENT);
    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsParticipatingInSeason()) && (event._keyCode == EngineAPI::KEY_1))
        {
            // TODO: Ideally figure out a better solution for opening the chat window than this...
            GameAPI::SendChatMessage(L"Server", L"Click on me to open the chat window", 2);
        }

        //if ((client.IsParticipatingInSeason()) && (event._keyCode == EngineAPI::KEY_TILDE))
        {
            // Disable the tilde key to prevent console access
            //return true;
        }
        //else
        {
            return callback(_this, event);
        }
    }
    return false;
}