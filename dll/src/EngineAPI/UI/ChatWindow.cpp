#include <vector>
#include <algorithm>
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
        instance.FindVisibleBit();
    }
    return instance;
}

void ChatWindow::ToggleDisplay()
{
    if (_visible != nullptr)
    {
        if (*_visible == 0)
        {
            if (!_prefix.empty())
            {
                wchar_t buffer[7] = { 0 };
                size_t length = min(_prefix.size(), 7);

                memcpy(buffer, _prefix.c_str(), sizeof(wchar_t) * length);
                memcpy(_visible + 0xB0, buffer, sizeof(wchar_t) * 7);
                *(_visible + 0xC0)  = length;  // String length
                *(_visible + 0xC8)  = 0x07;    // Some sort of buffer size value? <= 0x07 indicates in-place memory, >= 0x0F indicates a pointer that grows by 8 each time
                *(_visible + 0x160) = length;  // Text caret position
            }
        }

        *_visible ^= 1;
    }
}

bool ChatWindow::IsVisible()
{
    if (IsInitialized())
    {
        return (*_visible != 0);
    }
    return false;
}

inline bool CheckMagicAddress(uint8_t* buffer, uint64_t offset)
{
    return (((*(uint64_t*)(buffer + offset)) & 0x0000FFFF00000000) == 0x0000025800000000) && 
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 1))) & 0x000000000000FFFF) == 0x000000000000C350) &&
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 3))) & 0xFF00000000000000) == 0x4300000000000000) &&
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 4))) & 0xFF00000000000000) == 0x4100000000000000);
}

void ChatWindow::FindVisibleBit()
{
    typedef void* (*GetObjectManagerProto)();
    typedef void(__thiscall* GetObjectListProto)(void*, std::vector<void*>&);

    _visible = nullptr;

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
        void* min = *std::min_element(objects.begin(), objects.end());
        void* max = *std::max_element(objects.begin(), objects.end());

        uint8_t* start = (uint8_t*)min;
        uint64_t magic = 0x41c0000044008000;
        HANDLE process = GetCurrentProcess();
        MEMORY_BASIC_INFORMATION info;

        while ((_visible == nullptr) && (start < max))
        {
            if (VirtualQueryEx(process, start, &info, sizeof(info)) != sizeof(info))
                break;

            if ((info.RegionSize <= 0x400000) && (info.AllocationProtect & PAGE_READWRITE))
            {
                uint8_t* buffer = new uint8_t[info.RegionSize];
                ReadProcessMemory(process, info.BaseAddress, buffer, info.RegionSize, NULL);

                for (uint64_t offset = 0; (offset + sizeof(uint64_t)) <= info.RegionSize - (sizeof(uint64_t) * 4); offset += sizeof(uint64_t))
                {
                    if (CheckMagicAddress(buffer, offset))
                    {
                        _visible = (uint8_t*)info.BaseAddress + offset - 0x80;
                        break;
                    }
                }

                delete[] buffer;
            }
            start += info.RegionSize;
        }
    }
}

}