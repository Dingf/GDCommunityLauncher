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

bool ChatWindow::SetChatColor(ChatType type, uint32_t color, bool save)
{
    if (IsColorsInitialized())
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

        return true;
    }
    return false;
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
                *(_visible + 0xC0)  = (uint8_t)length;  // String length
                *(_visible + 0xC8)  = 0x07;             // Some sort of buffer size value? <= 0x07 indicates in-place memory, >= 0x0F indicates a pointer that grows by 8 each time
                *(_visible + 0x160) = (uint8_t)length;  // Text caret position
            }
        }

        *_visible ^= 1;
    }
}

bool ChatWindow::IsVisible()
{
    if (IsToggleInitialized())
    {
        return (*_visible != 0);
    }
    return false;
}

inline bool CheckVisibleBitAddress(uint8_t* buffer, uint64_t offset, uint64_t size)
{
    if (offset + 0x28 <= size)
    {
        return ((*(uint64_t*)(buffer + offset)        & 0x0000FFFF00000000) == 0x0000025800000000) &&
               ((*(uint64_t*)(buffer + offset + 0x08) & 0x000000000000FFFF) == 0x000000000000C350) &&
               ((*(uint64_t*)(buffer + offset + 0x18) & 0xFF000000000000FF) == 0x4300000000000000) &&
               ((*(uint64_t*)(buffer + offset + 0x20) & 0xF0000000000000FF) == 0x4000000000000000);
    }
    return false;
}

inline bool CheckVisibleBitAddress2(uint8_t* buffer, uint64_t offset, uint64_t size)
{
    if (offset + 0x50 <= size)
    {
        return (*(uint64_t*)(buffer + offset)        == 0x0075006F00720047) &&
               (*(uint64_t*)(buffer + offset + 0x08) == 0x0000003100200070) &&
               (*(uint64_t*)(buffer + offset + 0x20) == 0x0075006F00720047) &&
               (*(uint64_t*)(buffer + offset + 0x28) == 0x0000003200200070) &&
               (*(uint64_t*)(buffer + offset + 0x40) == 0x0075006F00720047) &&
               (*(uint64_t*)(buffer + offset + 0x48) == 0x0000003300200070);
    }
    return false;
}

inline bool CheckVisibleBitAddress3(uint8_t* buffer, uint64_t offset, uint64_t size)
{
    if (offset + 0x140 <= size)
    {
        return (*(buffer + offset) == 0x00) &&
               (*(buffer + offset + 0xC0) == 0x00) &&
               (*(buffer + offset + 0xC8) == 0x07) &&
               (*(buffer + offset + 0xD0) == 0x7C) &&
               (*(buffer + offset + 0x110) == 0x3A) &&
               (*(buffer + offset + 0x118) == 0x3F) &&
               (*(buffer + offset + 0x130) == 0x3A) &&
               (*(buffer + offset + 0x138) == 0x3F);
    }
    return false;
}

inline bool CheckColorAddress1(uint8_t* buffer, uint64_t offset, uint64_t size)
{
    if (offset + 0x68 <= size)
    {
        return (*(uint64_t*)(buffer + offset)        == 0x000000000061002F) &&
               (*(uint64_t*)(buffer + offset + 0x20) == 0x000000000070002F) &&
               (*(uint64_t*)(buffer + offset + 0x40) == 0x000000000072002F) &&
               (*(uint64_t*)(buffer + offset + 0x60) == 0x000000000074002F);
    }
    return false;
}

inline bool CheckColorAddress2(uint8_t* buffer, uint64_t offset, uint64_t size)
{
    if (offset + 0x40 <= size)
    {
        return (*(uint64_t*)(buffer + offset)        == 0x3F80000000000000) &&
               (*(uint64_t*)(buffer + offset + 0x08) == 0x3F80000000000000) &&
               (*(uint64_t*)(buffer + offset + 0x10) == 0x3F8000003F800000) &&
               (*(uint64_t*)(buffer + offset + 0x18) == 0x3F8000003F800000) &&
               (*(uint64_t*)(buffer + offset + 0x20) == 0x3F3333333F800000) &&
               (*(uint64_t*)(buffer + offset + 0x28) == 0x3F80000000000000) &&
               (*(uint64_t*)(buffer + offset + 0x30) == 0x3F80000000000000) &&
               (*(uint64_t*)(buffer + offset + 0x38) == 0x3F8000003F333333);
    }
    return false;
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

        while (start < max)
        {
            if (VirtualQueryEx(process, start, &info, sizeof(info)) != sizeof(info))
                break;

            if ((info.RegionSize <= 0x1000000) && (info.AllocationProtect & PAGE_READWRITE) && ((!_colors) || (!_visible)))
            {
                uint8_t* buffer = new uint8_t[info.RegionSize];
                ReadProcessMemory(process, info.BaseAddress, buffer, info.RegionSize, NULL);

                for (uint64_t offset = 0; (offset + sizeof(uint64_t)) <= info.RegionSize; offset += sizeof(uint64_t))
                {
                    if (!_visible)
                    {
                        if (CheckVisibleBitAddress(buffer, offset, info.RegionSize))
                            _visible = (uint8_t*)info.BaseAddress + offset - 0x80;
                        else if (CheckVisibleBitAddress2(buffer, offset, info.RegionSize))
                            _visible = (uint8_t*)info.BaseAddress + offset + 0xAF8;
                        else if (CheckVisibleBitAddress3(buffer, offset, info.RegionSize))
                            _visible = (uint8_t*)info.BaseAddress + offset;

                        // The visible bit should be 0 initially, so if it's not then reset the address
                        if ((_visible) && (*_visible != 0))
                            _visible = nullptr;
                    }
                    if (!_colors)
                    {
                        if (CheckColorAddress1(buffer, offset, info.RegionSize))
                            _colors = (uint8_t*)info.BaseAddress + offset + 0xE0;
                        else if (CheckColorAddress2(buffer, offset, info.RegionSize))
                            _colors = (uint8_t*)info.BaseAddress + offset;
                    }
                }
                delete[] buffer;
            }
            start += info.RegionSize;
        }
    }

    if (_colors)
    {
        uint64_t* tradeShortcut = (uint64_t*)(_colors - 0x80);
        if (*tradeShortcut == 0x000000000074002F)
            *tradeShortcut = 0x000000650074002F;   // Change /t to /te so that we can bind /t ourselves
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

        if (IsColorsInitialized())
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