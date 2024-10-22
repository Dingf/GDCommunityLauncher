#include <vector>
#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void SetCloudStorageEnabled(bool enabled)
{
    typedef void (__thiscall* SetCloudStorageEnabledProto)(void*, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    SetCloudStorageEnabledProto callback = (SetCloudStorageEnabledProto)GetProcAddress(gameDLL, GAPI_NAME_SET_CLOUD_STORAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, enabled);
}

bool IsCloudStorageEnabled()
{
    typedef bool(__thiscall* IsCloudStorageEnabledProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
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

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
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

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    SaveGameProto callback = (SaveGameProto)GetProcAddress(gameDLL, GAPI_NAME_SAVE_GAME);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    return callback(*gameEngine);
}

void SaveTransferStash()
{
    typedef void (__thiscall* SaveTransferStashProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    SaveTransferStashProto callback = (SaveTransferStashProto)GetProcAddress(gameDLL, GAPI_NAME_SAVE_TRANSFER_STASH);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine);
}

void LoadTransferStash()
{
    typedef void (__thiscall* LoadTransferStashProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    LoadTransferStashProto callback = (LoadTransferStashProto)GetProcAddress(gameDLL, GAPI_NAME_LOAD_TRANSFER_STASH);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine);
}

void SetNumberOfTransferTabs(uint32_t amount)
{
    typedef void (__thiscall* SetNumberOfTransferTabsProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    SetNumberOfTransferTabsProto callback = (SetNumberOfTransferTabsProto)GetProcAddress(gameDLL, GAPI_NAME_RESTORE_NUMBER_OF_TRANSFER_SACKS);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, amount);
}

void DisplayCaravanWindow(uint32_t caravanID)
{
    typedef void (__thiscall* DisplayCaravanWindowProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    DisplayCaravanWindowProto callback = (DisplayCaravanWindowProto)GetProcAddress(gameDLL, GAPI_NAME_DISPLAY_CARAVAN_WINDOW);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine) || (IsCaravanWindowOpen()))
        return;

    callback(*gameEngine, caravanID);
}

bool IsCaravanWindowOpen()
{
    void** gameEngine = GetGameEngineHandle();
    uint32_t caravanID = *(uint32_t*)((uintptr_t)(*gameEngine) + 0x35B08);
    return (caravanID != 0);
}

void DisplayUINotification(const std::string& tag)
{
    typedef void(__thiscall* DisplayUINotificationProto)(void*, const std::string&);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    DisplayUINotificationProto callback = (DisplayUINotificationProto)GetProcAddress(gameDLL, GAPI_NAME_UI_NOTIFY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    return callback(*gameEngine, tag);
}

void SendChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type)
{
    typedef void(__thiscall* SendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    void* mainPlayer = GameAPI::GetMainPlayer();
    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!mainPlayer))
        return;

    uint32_t playerID = GetPlayerPartyID(mainPlayer);

    SendChatMessageProto callback = (SendChatMessageProto)GetProcAddress(gameDLL, GAPI_NAME_SEND_CHAT_MESSAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, name, message, type, { playerID }, 0);
}

void AddChatMessage(const std::wstring& name, const std::wstring& message, uint8_t type, void* item)
{
    typedef void(__thiscall* AddChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    AddChatMessageProto callback = (AddChatMessageProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_CHAT_MESSAGE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine, name, message, type, item);
}

void ReloadDatabase()
{
    typedef void (__thiscall* ReloadDatabaseProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    ReloadDatabaseProto callback = (ReloadDatabaseProto)GetProcAddress(gameDLL, GAPI_NAME_RELOAD_DATABASE);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine);
}

}