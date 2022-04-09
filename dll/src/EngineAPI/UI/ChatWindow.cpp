#include <vector>
#include <Windows.h>
#include "EngineAPI.h"
#include "EngineAPI/UI/ChatWindow.h"

namespace EngineAPI::UI
{

ChatWindow& ChatWindow::GetInstance(bool init)
{
    static ChatWindow instance;
    if (init)
    {
        instance._visible = nullptr;
        instance.FindVisibleBit();
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

void ChatWindow::FindVisibleBit()
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
    switch (EngineAPI::GetPlatform())
    {
        case EngineAPI::PLATFORM_STEAM:
            magic = 0x7ff7ec1f87a8;
            break;
        case EngineAPI::PLATFORM_GOG:
            magic = 0x7ff68fa330e8;
            break;
    }

    if ((objects.size() > 0) && (magic > 0))
    {
        void* min = objects[0];
        void* max = objects[0];
        for (size_t i = 1; i < objects.size(); ++i)
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