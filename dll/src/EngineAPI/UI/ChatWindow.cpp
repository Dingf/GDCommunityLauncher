#include <vector>
#include <algorithm>
#include <filesystem>
#include <Windows.h>
#include "EngineAPI.h"
#include "EngineAPI/UI/ChatWindow.h"
#include "Configuration.h"

namespace EngineAPI::UI
{

ChatWindow& ChatWindow::GetInstance(bool init)
{
    static ChatWindow instance;
    if (init)
    {
        instance.FindMagicAddresses();
        instance.LoadConfig();
    }
    return instance;
}

uint32_t ChatWindow::GetChatColor(ChatType type)
{
    switch (type)
    {
        case CHAT_TYPE_TRADE:
            return _tradeColor;
        case CHAT_TYPE_GLOBAL:
            return _globalColor;
        default:
            return 0xFFFFFFFF;
    }
}

void ChatWindow::SetChatColor(ChatType type, uint32_t color, bool save)
{
    float* baseAddress = nullptr;

    // Trim alpha since we will always set it to 1.0f
    color &= 0x00FFFFFF;

    if (type == CHAT_TYPE_TRADE)
    {
        baseAddress = (float*)(_colors);
        _tradeColor = color;
    }
    else if (type == CHAT_TYPE_GLOBAL)
    {
        baseAddress = (float*)(_colors + sizeof(uint64_t) * 4);
        _globalColor = color;
    }

    if (baseAddress)
    {
        baseAddress[0] = (color & 0x0000FF) / 255.0f;
        baseAddress[1] = ((color & 0x00FF00) >> 8) / 255.0f;
        baseAddress[2] = ((color & 0xFF0000) >> 16) / 255.0f;
        baseAddress[3] = 1.0f;
    }

    if (save)
        SaveConfig();
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
                uint8_t length = min(_prefix.size(), 7);

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

inline bool CheckVisibleBitAddress(uint8_t* buffer, uint64_t offset)
{
    return (((*(uint64_t*)(buffer + offset))                          & 0x0000FFFF00000000) == 0x0000025800000000) && 
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 1))) & 0x000000000000FFFF) == 0x000000000000C350) &&
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 2)))                     ) == 0x0000000000000000) &&
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 3))) & 0xFF000000000000FF) == 0x4300000000000000) &&
           (((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 4))) & 0xF0000000000000FF) == 0x4000000000000000);
}

inline bool CheckColorAddress(uint8_t* buffer, uint64_t offset)
{
    return ((*(uint64_t*)(buffer + offset))                           == 0x000000000061002F) &&
           ((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 4)))  == 0x000000000070002F) && 
           ((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 8)))  == 0x000000000072002F) &&
           ((*(uint64_t*)(buffer + offset + (sizeof(uint64_t) * 12))) == 0x000000000074002F);
}

void ChatWindow::FindMagicAddresses()
{
    typedef void* (*GetObjectManagerProto)();
    typedef void(__thiscall* GetObjectListProto)(void*, std::vector<void*>&);

    _visible = nullptr;
    _colors = nullptr;

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

            if ((info.RegionSize <= 0x400000) && (info.AllocationProtect & PAGE_READWRITE) && ((!_colors) || (!_visible)))
            {
                uint8_t* buffer = new uint8_t[info.RegionSize];
                ReadProcessMemory(process, info.BaseAddress, buffer, info.RegionSize, NULL);

                for (uint64_t offset = 0; (offset + sizeof(uint64_t)) <= info.RegionSize - (sizeof(uint64_t) * 4); offset += sizeof(uint64_t))
                {
                    if ((!_visible) && (CheckVisibleBitAddress(buffer, offset)))
                    {
                        _visible = (uint8_t*)info.BaseAddress + offset - 0x80;
                    }
                    if ((!_colors) && (CheckColorAddress(buffer, offset)))
                    {
                        _colors = (uint8_t*)info.BaseAddress + offset + 0xE0;
                        (*(uint64_t*)((uint8_t*)info.BaseAddress + offset + (sizeof(uint64_t) * 0x0C))) = 0x000000650074002F;   // Change /t to /te so that we can bind /t ourselves
                    }
                }
                delete[] buffer;
            }
            start += info.RegionSize;
        }
    }
}

void ChatWindow::LoadConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);

        const Value* globalColorValue = config.GetValue("Chat", "global_color");
        _globalColor = (globalColorValue) ? globalColorValue->ToInt() : EngineAPI::Color::ORANGE.GetColorCode();

        const Value* tradeColorValue = config.GetValue("Chat", "trade_color");
        _tradeColor = (tradeColorValue) ? tradeColorValue->ToInt() : EngineAPI::Color::GREEN.GetColorCode();

        if (_colors)
        {
            SetChatColor(CHAT_TYPE_GLOBAL, _globalColor, false);
            SetChatColor(CHAT_TYPE_TRADE, _tradeColor, false);
            SaveConfig();
        }
    }
}

void ChatWindow::SaveConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        config.SetValue("Chat", "global_color", (int)(_globalColor & 0x00FFFFFF));
        config.SetValue("Chat", "trade_color", (int)(_tradeColor & 0x00FFFFFF));
        config.Save(configPath);
    }
}

}