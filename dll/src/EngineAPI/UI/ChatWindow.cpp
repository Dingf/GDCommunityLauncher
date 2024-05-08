#include <vector>
#include <algorithm>
#include <filesystem>
#include <Windows.h>
#include "EngineAPI.h"
#include "EngineAPI/UI/ChatWindow.h"
#include "GameAPI/GameEngine.h"
#include "ThreadManager.h"
#include "Configuration.h"

namespace EngineAPI::UI
{

ChatWindow& ChatWindow::GetInstance()
{
    static ChatWindow instance;
    if (!instance.IsInitialized())
    {
        instance.Initialize();
    }
    return instance;
}

void ChatWindow::Initialize()
{
    FindMagicAddresses();
    LoadConfig();
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
    if (IsInitialized())
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
        case EngineAPI::Input::KEY_TAB:
        case EngineAPI::Input::KEY_ESC:
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
        {
            if (!IsVisible())
            {
                ToggleDisplay();
                return true;
            }
            return false;
        }
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
    if (IsInitialized())
    {
        return (*_visible != 0);
    }
    return false;
}

void ChatWindow::FindMagicAddresses()
{
    void* gameEngine = *GameAPI::GetGameEngineHandle();
    if (gameEngine)
    {
        _visible = *(uint8_t**)((uint8_t*)gameEngine + 0x18A0) + 0x45F90;
        _colors = _visible + 0x2C28;

        // Change the in-game command names to avoid conflicts with the launcher chat commands
        *(std::wstring*)(_colors - 0x120) = L"_Mute";
        *(std::wstring*)(_colors - 0x100) = L"_Unmute";
        *(std::wstring*)(_colors - 0x80) = L"_t";
        *(std::wstring*)(_colors - 0x60) = L"_m";
        *(std::wstring*)(_colors - 0x40) = L"_u";
    }
    else
    {
        _visible = nullptr;
        _colors = nullptr;
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

        if (IsInitialized())
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