#include <cwctype>
#include <filesystem>
#include "ChatAPI.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "EventManager.h"
#include "Configuration.h"
#include "JSON.h"
#include "Log.h"

namespace ChatAPI
{

bool _initialized = false; // Whether the chat API has been initialized yet
uint8_t _channel;          // Current global chat channel
uint8_t* _visibleAddress;  // Address used to toggle the chat window visibility
uint8_t* _colorAddress;    // Address used to set the chat colors
std::wstring _prefix;      // Last used chat prefix
std::wstring _saved;       // Saved buffer text, used for linking items in chat
std::wstring _empty;       // Empty buffer text; used by GetBufferText() if the chat window isn't initialized yet
std::unordered_set<std::wstring> _mutedList;    // List of muted players by the current user

uint32_t GetChatColor(ChatType type)
{
    if (_colorAddress)
    {
        float* baseAddress = nullptr;
        if (type == CHAT_TYPE_SYSTEM)
        {
            baseAddress = (float*)(_colorAddress);
        }
        else if (type == CHAT_TYPE_GLOBAL)
        {
            baseAddress = (float*)(_colorAddress + sizeof(uint64_t) * 4);
        }

        if (baseAddress)
        {
            uint32_t r = baseAddress[0] * 255;
            uint32_t g = baseAddress[1] * 255;
            uint32_t b = baseAddress[2] * 255;
            return 0xFF000000 | ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | (r & 0xFF);
        }
    }
    return EngineAPI::Color::WHITE.GetColorCode();
}

uint8_t GetChatChannel()
{
    return _channel;
}

const std::wstring& GetChatPrefix()
{
    return _prefix;
}

const std::wstring& GetBufferText()
{
    return (_visibleAddress) ? *(std::wstring*)(_visibleAddress + 0xB0) : _empty;
}

const std::wstring& GetSavedText()
{
    return _saved;
}

uint32_t GetCaratPosition()
{
    return (_visibleAddress) ? *(uint32_t*)(_visibleAddress + 0x160) : 0;
}

uint32_t GetSelectStartPosition()
{
    return (_visibleAddress) ? *(uint32_t*)(_visibleAddress + 0x164) : 0;
}

uint32_t GetSelectEndPosition()
{
    return (_visibleAddress) ? *(uint32_t*)(_visibleAddress + 0x168) : 0;
}

const std::unordered_set<std::wstring>& GetMutedList()
{
    return _mutedList;
}

bool IsPlayerMuted(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    return (_mutedList.count(playerName) > 0);
}

bool IsWindowVisible()
{
    return (_visibleAddress) ? (*_visibleAddress != 0) : false;
}

bool SetChatColor(ChatType type, uint32_t color)
{
    if (_colorAddress)
    {
        float* baseAddress = nullptr;

        // Trim alpha since we will always set it to 1.0f
        color &= 0x00FFFFFF;

        if (type == CHAT_TYPE_SYSTEM)
        {
            baseAddress = (float*)(_colorAddress);
        }
        else if (type == CHAT_TYPE_GLOBAL)
        {
            baseAddress = (float*)(_colorAddress + sizeof(uint64_t) * 4);
        }

        if (baseAddress)
        {
            baseAddress[0] = (color & 0x0000FF) / 255.0f;
            baseAddress[1] = ((color & 0x00FF00) >> 8) / 255.0f;
            baseAddress[2] = ((color & 0xFF0000) >> 16) / 255.0f;
            baseAddress[3] = 1.0f;
        }
        return true;
    }
    return false;
}

void SetChatChannel(uint8_t channel)
{
    _channel = channel;
}

void SetCaratPosition(uint32_t position)
{
    if (_visibleAddress)
    {
        const std::wstring& bufferText = GetBufferText();
        position = (position > bufferText.size()) ? bufferText.size() : position;
        *(uint32_t*)(_visibleAddress + 0x160) = position;
    }
}

void SetSelectStartPosition(uint32_t position)
{
    if (_visibleAddress)
    {
        const std::wstring& bufferText = GetBufferText();
        position = (position > bufferText.size()) ? bufferText.size() : position;
        *(uint32_t*)(_visibleAddress + 0x164) = position;
    }
}

void SetSelectEndPosition(uint32_t position)
{
    if (_visibleAddress)
    {
        const std::wstring& bufferText = GetBufferText();
        position = (position > bufferText.size()) ? bufferText.size() : position;
        *(uint32_t*)(_visibleAddress + 0x168) = position;
    }
}

void SetChatPrefix(const std::wstring& prefix)
{
    _prefix = prefix;
}

void SetBufferText(const std::wstring& text)
{
    if (_visibleAddress)
    {
        std::wstring trimmedText = (text.size() >= MAX_MESSAGE_SIZE) ? text.substr(0, MAX_MESSAGE_SIZE) : text;
        std::wstring& bufferText = *(std::wstring*)(_visibleAddress + 0xB0);
        uint32_t& caratPosition = *(uint32_t*)(_visibleAddress + 0x160);

        bufferText = trimmedText;
        caratPosition = (uint32_t)trimmedText.size();
    }
}

void SaveBufferText()
{
    _saved = GetBufferText();
}

bool MutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    if (!_mutedList.contains(playerName))
    {
        _mutedList.insert(playerName);
        return true;
    }
    else
    {
        return false;
    }
}

bool UnmutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    auto it = _mutedList.find(playerName);
    if (it != _mutedList.end())
    {
        _mutedList.erase(it);
        return true;
    }
    else
    {
        return false;
    }
}

void ClearMutedList()
{
    _mutedList.clear();
}

void ToggleWindowDisplay()
{
    if (_visibleAddress != nullptr)
    {
        if ((*_visibleAddress == 0) && (!_prefix.empty()))
            SetBufferText(_prefix);

        *_visibleAddress ^= 1;
        *(_visibleAddress + 0xA9) = *_visibleAddress;
    }
}

void FindMagicAddresses()
{
    void* gameEngine = *GameAPI::GetGameEngineHandle();
    if (gameEngine)
    {
        std::string versionString = EngineAPI::GetVersionString();

        //_visibleAddress = *(uint8_t**)((uint8_t*)gameEngine + 0x18A0) + 0x45F90;    // Pre-version 1.2.0.5
        //_visibleAddress = *(uint8_t**)((uint8_t*)gameEngine + 0x18B0) + 0x45BD8;    // Version 1.2.0.5
        _visibleAddress = *(uint8_t**)((uint8_t*)gameEngine + 0x18C0) + 0x45BD8;      // Version 1.2.1.3

        _colorAddress = _visibleAddress + 0x2C28;

        // Change the in-game command names to avoid conflicts with the launcher chat commands
        *(std::wstring*)(_colorAddress - 0x120) = L"_Mute";
        *(std::wstring*)(_colorAddress - 0x100) = L"_Unmute";
        *(std::wstring*)(_colorAddress - 0x80) = L"_t";
        *(std::wstring*)(_colorAddress - 0x60) = L"_m";
        *(std::wstring*)(_colorAddress - 0x40) = L"_u";
    }
    else
    {
        _visibleAddress = nullptr;
        _colorAddress = nullptr;
    }
}

void LoadConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);

        const Value* channelValue = config.GetValue("Chat", "channel");
        SetChatChannel((channelValue) ? channelValue->ToInt() : 0);

        const Value* systemColorValue = config.GetValue("Chat", "system_color");
        SetChatColor(CHAT_TYPE_SYSTEM, (systemColorValue) ? systemColorValue->ToInt() : EngineAPI::Color::FUSHIA.GetColorCode());

        const Value* globalColorValue = config.GetValue("Chat", "global_color");
        SetChatColor(CHAT_TYPE_GLOBAL, (globalColorValue) ? globalColorValue->ToInt() : EngineAPI::Color::ORANGE.GetColorCode());
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

void SaveConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        config.SetValue("Chat", "channel", (int)_channel);
        config.SetValue("Chat", "system_color", (int)(GetChatColor(CHAT_TYPE_SYSTEM) & 0x00FFFFFF));
        config.SetValue("Chat", "global_color", (int)(GetChatColor(CHAT_TYPE_GLOBAL) & 0x00FFFFFF));
        config.Save(configPath);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

void OnSetMainPlayerEvent(void* player)
{
    FindMagicAddresses();
    LoadConfig();
}

void OnWorldPreUnloadEvent()
{
    SaveConfig();
    _visibleAddress = nullptr;
    _colorAddress = nullptr;
}

bool Initialize()
{
    if (!_initialized)
    {
        EventManager::Subscribe(GDCL_EVENT_SET_MAIN_PLAYER,  OnSetMainPlayerEvent);
        EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD, OnWorldPreUnloadEvent);
        _initialized = true;
        return true;
    }
    return false;
}

}