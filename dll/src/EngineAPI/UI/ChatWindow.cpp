#include <vector>
#include <Windows.h>
#include "EngineAPI.h"
#include "EngineAPI/UI/ChatWindow.h"
#include "Log.h"

namespace EngineAPI::UI
{

ChatWindow& ChatWindow::GetInstance(bool init)
{
    static ChatWindow instance;
    if (init)
    {
        instance._visible = nullptr;
        instance.FindMagicBit();
    }
    return instance;
}

void ChatWindow::ToggleDisplay()
{
    if (_visible != nullptr)
    {
        *_visible ^= 1;
    }
}

bool ChatWindow::GetState()
{
    if (_visible != nullptr)
    {
        return (*_visible != 0);
    }
    return false;
}

void ChatWindow::FindMagicBit()
{
    typedef void* (*GetObjectManagerProto)();
    typedef void(__thiscall* GetObjectListProto)(void*, std::vector<void*>&);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return;

    GetObjectManagerProto GetObjectManager = (GetObjectManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_MANAGER);
    GetObjectListProto GetObjectList = (GetObjectListProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_LIST);
    if (!GetObjectManager || !GetObjectList)
        return;

    std::vector<void*> objects;
    GetObjectList(GetObjectManager(), objects);

    uint64_t magic = 0;
    if (EngineAPI::IsUsingSteam())
        //magic = 0x7ff6bdda87a8;
        magic = 0x7ff7ec1f87a8;
    else if (EngineAPI::IsUsingGOG())
        magic = 0x7ff68fa330e8;

    Logger::LogMessage(LOG_LEVEL_DEBUG, "DEBUG % %", EngineAPI::IsUsingGOG(), EngineAPI::IsUsingSteam());

    if ((objects.size() > 0) && (magic > 0))
    {
        void* min = objects[0];
        void* max = objects[0];
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (objects[i] < min)
                min = objects[i];
            if (objects[i] > max)
                max = objects[i];
        }

        for (LPVOID* current = (LPVOID*)min; current < max; ++current)
        {
            if (!IsBadReadPtr(current, 8) && (*current == (LPVOID)magic))
            {
                _visible = (uint8_t*)current - 0x28;
                return;
            }
        }
    }
}

}