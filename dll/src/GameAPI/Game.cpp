#include <vector>
#include <Windows.h>
#include "GameAPI/GameEngine.h"
#include "GameAPI/Difficulty.h"
#include "GameAPI/Game.h"
#include "GameAPI/Player.h"

namespace GameAPI
{

bool IsCloudStorageEnabled()
{
    typedef bool(__thiscall* IsCloudStorageEnabledProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    IsCloudStorageEnabledProto callback = (IsCloudStorageEnabledProto)GetProcAddress(gameDLL, GAPI_NAME_IS_CLOUD_STORAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback(*gameEngine);
}

bool IsGameLoading()
{
    typedef bool(__thiscall* IsGameLoadingProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    IsGameLoadingProto callback = (IsGameLoadingProto)GetProcAddress(gameDLL, GAPI_NAME_IS_GAME_LOADING);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return false;

    return callback(*gameEngine);
}

void SaveGame()
{
    typedef void(__thiscall* SaveGameProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return;

    SaveGameProto callback = (SaveGameProto)GetProcAddress(gameDLL, GAPI_NAME_SAVE_GAME);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    return callback(*gameEngine);
}

void DisplayUINotification(const std::string& tag)
{
    typedef void(__thiscall* DisplayUINotificationProto)(void*, const std::string&);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return;

    DisplayUINotificationProto callback = (DisplayUINotificationProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_UI_NOTIFY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    return callback(*gameEngine, tag);
}

void SendChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type)
{
    typedef void(__thiscall* SendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    void* mainPlayer = GameAPI::GetMainPlayer();
    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!mainPlayer))
        return;

    uint32_t playerID = GetPlayerPartyID(mainPlayer);

    SendChatMessageProto callback = (SendChatMessageProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, name, message, type, { playerID }, 0);
}

void AddChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type, void* item)
{
    typedef void(__thiscall* AddChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return;

    AddChatMessageProto callback = (AddChatMessageProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_ADD_CHAT_MESSAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, name, message, type, item);
}

}