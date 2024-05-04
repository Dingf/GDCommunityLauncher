#include <vector>
#include <algorithm>
#include <filesystem>
#include <Windows.h>
#include "EngineAPI.h"
#include "EngineAPI/UI/ChatWindow.h"
#include "ThreadManager.h"
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

uint32_t ChatWindow::GetChatColor(ChatType type) const
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

// Special paste handler, since regular paste won't exceed the chat window length
bool HandlePasteEvent(std::wstring& text, uint32_t& carat, uint32_t& selectStart, uint32_t& selectEnd)
{
    if (OpenClipboard(nullptr))
    {
        if (HANDLE data = GetClipboardData(CF_TEXT))
        {
            if (char* charData = (char*)GlobalLock(data))
            {
                int32_t select = (selectEnd >= selectStart) ? selectEnd - selectStart : 0;
                size_t length = text.size() - select;
                std::wstring pasteText;
                while ((*charData != '\0') && (length < MAX_CHAT_SIZE))
                {
                    pasteText.push_back(*charData);
                    charData++;
                    length++;
                }

                if (select > 0)
                {
                    text.erase(selectStart, select);
                    carat = selectStart;
                    selectStart = 0;
                    selectEnd = 0;
                }

                text.insert(carat, pasteText);
                carat += (uint32_t)pasteText.size();

                GlobalUnlock(data);
                CloseClipboard();
                return true;
            }
        }
    }
    return false;
}

void ChatWindow::HoldKeyUpdate()
{
    ChatWindow& chatWindow = GetInstance();
    if (chatWindow._holdEvent._key != EngineAPI::Input::KEY_NONE)
    {
        if (!chatWindow._holdLock)
            chatWindow.HandleKeyPress(chatWindow._holdEvent);
    }
}

typedef bool (*ControlKeyHandler)(std::wstring&, uint32_t&, uint32_t&, uint32_t&);
const std::map<EngineAPI::Input::KeyCode, ControlKeyHandler> controlKeyHandlers =
{
    { EngineAPI::Input::KEY_V, &HandlePasteEvent },
};


bool ChatWindow::HandleKeyPress(EngineAPI::Input::KeyButtonEvent& event)
{
    std::wstring& text = *(std::wstring*)(_visible + 0xB0);
    uint32_t& carat = GetCaratPosition();
    uint32_t& selectStart = GetSelectStartPosition();
    uint32_t& selectEnd = GetSelectEndPosition();

    switch (event._key)
    {
        case EngineAPI::Input::KEY_ESC:
            // ESC produces output for some reason, but let the main program handle it
            return false;
        case EngineAPI::Input::KEY_BACKSPACE:
            if (IsVisible())
            {
                int32_t select = selectEnd - selectStart;
                if (select > 0)
                {
                    text.erase(selectStart, select);
                    carat = selectStart;
                    selectStart = 0;
                    selectEnd = 0;
                }
                else if (carat > 0)
                {
                    text.erase(--carat, 1);
                }
                return true;
            }
            return false;
        case EngineAPI::Input::KEY_ENTER:
            // Only handle the initial toggle behavior; when sending a message, let the main program handle it
            if ((IsToggleInitialized()) && (!IsVisible()))
            {
                ToggleDisplay();
                return true;
            }
            return false;
        default:
        {
            if ((IsVisible()) && (event._output != 0) && (text.size() < MAX_CHAT_SIZE))
            {
                // Ctrl + key usually has output, but shouldn't actually print a character to the window
                // Most of the time, just let the main program handle it
                if (event._modifier & EngineAPI::Input::KEY_MODIFIER_CTRL)
                {
                    auto pair = controlKeyHandlers.find(event._key);
                    if (pair != controlKeyHandlers.end())
                    {
                        ControlKeyHandler handler = pair->second;
                        return handler(text, carat, selectStart, selectEnd);
                    }
                    return false;
                }

                int32_t select = selectEnd - selectStart;
                if (select > 0)
                {
                    text.erase(selectStart, select);
                    carat = selectStart;
                    selectStart = 0;
                    selectEnd = 0;
                }

                text.insert(carat, 1, (wchar_t)event._output);
                carat++;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool ChatWindow::HandleKeyEvent(EngineAPI::Input::KeyButtonEvent& event)
{
    bool result = false;
    if (event._state == EngineAPI::Input::KEY_STATE_DOWN)
    {
        _holdLock = true;
        _holdEvent = event;
        ThreadManager::CreatePeriodicThread("chat_window_hold", 50, 1, 500, &ChatWindow::HoldKeyUpdate);
        result = HandleKeyPress(event);
        _holdLock = false;
    }
    else
    {
        _holdEvent._key = EngineAPI::Input::KEY_NONE;
        ThreadManager::StopThread("chat_window_hold");
    }
    return result;
}

void ChatWindow::SetCaratPosition(uint32_t position)
{
    *(uint32_t*)(_visible + 0x160) = position;
}

void ChatWindow::SetSelectStartPosition(uint32_t position)
{
    *(uint32_t*)(_visible + 0x164) = position;
}

void ChatWindow::SetSelectEndPosition(uint32_t position)
{
    *(uint32_t*)(_visible + 0x168) = position;
}

void ChatWindow::SetBufferText(const std::wstring& text)
{
    std::wstring bufferText = (text.size() >= MAX_CHAT_SIZE) ? text.substr(0, MAX_CHAT_SIZE) : text;
    *(std::wstring*)(_visible + 0xB0) = bufferText;
    SetCaratPosition((uint32_t)bufferText.size());
}

void ChatWindow::ToggleDisplay()
{
    if (_visible != nullptr)
    {
        if ((*_visible == 0) && (!_prefix.empty()))
            SetBufferText(_prefix);

        *_visible ^= 1;
        *(_visible + 0xA9) = *_visible;
    }
}

bool ChatWindow::IsVisible() const
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
    _visible = nullptr;
    _colors = nullptr;

    std::vector<void*> objects;
    EngineAPI::GetObjectList(objects);
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
        // Change the in-game command names to avoid conflicts with the launcher chat commands
        *(std::wstring*)(_colors - 0x120) = L"_Mute";
        *(std::wstring*)(_colors - 0x100) = L"_Unmute";
        *(std::wstring*)(_colors - 0x80) = L"_t";
        *(std::wstring*)(_colors - 0x60) = L"_m";
        *(std::wstring*)(_colors - 0x40) = L"_u";
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