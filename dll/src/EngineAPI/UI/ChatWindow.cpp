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
        instance._magic = nullptr;
        instance.FindMagicBit();
    }
    return instance;
}

void ChatWindow::ToggleDisplay()
{
    if (_magic != nullptr)
    {
        *_magic ^= 1;
    }
}

bool ChatWindow::GetState()
{
    if (_magic != nullptr)
    {
        return (*_magic != 0);
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

    if (objects.size() > 0)
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
            if (!IsBadReadPtr(current, 8) && (*current == (LPVOID)0x7ff6bdda87a8))
            {
                _magic = (uint8_t*)current - 0x28;
                return;
            }
        }
    }
}

}