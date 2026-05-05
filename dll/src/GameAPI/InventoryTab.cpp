#include <atomic>
#include <map>
#include <vector>
#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

bool AddItemToTab(void* tab, const EngineAPI::Vec2& position, void* item, bool unk1)
{
    typedef bool (__thiscall* AddItemToTabProto)(void*, const EngineAPI::Vec2&, void*, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    AddItemToTabProto callback = (AddItemToTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_ADD_ITEM_1);
    if ((!callback) || (!tab))
        return false;

    return callback(tab, position, item, unk1);
}

bool AddItemToTab(void* tab, void* item, bool unk1, bool unk2)
{
    typedef bool (__thiscall* AddItemToTabProto)(void*, void*, bool, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    AddItemToTabProto callback = (AddItemToTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_ADD_ITEM_2);
    if ((!callback) || (!tab))
        return false;

    return callback(tab, item, unk1, unk2);
}

bool RemoveItemFromTab(void* tab, uint32_t itemID)
{
    typedef bool (__thiscall* RemoveItemFromTabProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    RemoveItemFromTabProto callback = (RemoveItemFromTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_REMOVE_ITEM);
    if ((!callback) || (!tab))
        return false;

    return callback(tab, itemID);
}

void RemoveAllItemsFromTab(void* tab)
{
    typedef void (__thiscall* RemoveAllItemsFromTabProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    RemoveAllItemsFromTabProto callback = (RemoveAllItemsFromTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_REMOVE_ALL_ITEMS);
    if ((!callback) || (!tab))
        return;

    callback(tab);
}

const std::map<std::string, ReagentData>& GetPlayerReagents()
{
    static std::map<std::string, ReagentData> empty;
    typedef const std::map<std::string, ReagentData>& (__thiscall* GetPlayerReagentsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return empty;

    GetPlayerReagentsProto callback = (GetPlayerReagentsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_REAGENTS);
    void** gameEngine = GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return empty;

    return callback(*gameEngine);
}

const std::map<uint32_t, EngineAPI::Rect>& GetItemsInTab(void* tab)
{
    static std::map<uint32_t, EngineAPI::Rect> empty;
    typedef const std::map<uint32_t, EngineAPI::Rect>& (__thiscall* GetItemsInTabProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return empty;

    GetItemsInTabProto callback = (GetItemsInTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_GET_ITEMS);
    if ((!callback) || (!tab))
        return empty;

    return callback(tab);
}

bool AddItemToTransfer(uint32_t tab, const EngineAPI::Vec2& position, uint32_t itemID, bool unk1)
{
    typedef bool (__thiscall* AddItemToTransferProto)(void*, uint32_t, const EngineAPI::Vec2&, uint32_t, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    AddItemToTransferProto callback = (AddItemToTransferProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TRANSFER_ADD_ITEM_1);
    void** gameEngine = GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return false;

    return callback(*gameEngine, tab, position, itemID, unk1);
}

bool AddItemToTransfer(uint32_t itemID, uint32_t tab, bool unk1)
{
    typedef bool (__thiscall* AddItemToTransferProto)(void*, uint32_t, uint32_t, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    AddItemToTransferProto callback = (AddItemToTransferProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TRANSFER_ADD_ITEM_2);
    void** gameEngine = GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return false;

    return callback(*gameEngine, itemID, tab, unk1);
}

void* GetPlayerInventory(void* player)
{
    typedef void* (__thiscall* GetPlayerInventoryProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetPlayerInventoryProto callback = (GetPlayerInventoryProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_INVENTORY);
    void* playerController = GetPlayerController(player);
    if ((!callback) || (!playerController))
        return nullptr;

    return callback(playerController);
}

void* GetPlayerEquipment(void* player)
{
    typedef void* (__thiscall* GetPlayerEquipmentProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetPlayerEquipmentProto callback = (GetPlayerEquipmentProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_EQUIPMENT);
    void* playerController = GetPlayerController(player);

    if ((!callback) || (!playerController))
        return nullptr;

    return callback(playerController);
}

void* GetTransferStash(void* player)
{
    typedef void* (__thiscall* GetPlayerEquipmentProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetPlayerEquipmentProto callback = (GetPlayerEquipmentProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_EQUIPMENT);
    void* playerController = GetPlayerController(player);

    if ((!callback) || (!playerController))
        return nullptr;

    return callback(playerController);
}

void* GetInventoryTab(void* inventory, int32_t index)
{
    typedef void* (__thiscall* GetInventoryTabProto)(void*, int32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetInventoryTabProto callback = (GetInventoryTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_INVENTORY_TAB);
    if ((!callback) || (!inventory))
        return nullptr;

    return callback(inventory, index);
}

uint32_t GetInventoryTabCount(void* inventory)
{
    typedef uint32_t(__thiscall* GetInventoryTabCountProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetInventoryTabCountProto callback = (GetInventoryTabCountProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_INVENTORY_TAB_COUNT);
    if ((!callback) || (!inventory))
        return 0;

    return callback(inventory);
}

const std::vector<void*>& GetPersonalTabs(void* player)
{
    static std::vector<void*> empty;
    typedef const std::vector<void*>& (__thiscall* GetPersonalTabsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return empty;

    GetPersonalTabsProto callback = (GetPersonalTabsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PERSONAL_TABS);
    if ((!callback) || (!player))
        return empty;

    return callback(player);
}

const std::vector<void*>& GetTransferTabs()
{
    static std::vector<void*> empty;
    typedef const std::vector<void*>& (__thiscall* GetTransferTabsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return empty;

    GetTransferTabsProto callback = (GetTransferTabsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_TRANSFER_TABS);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return empty;

    return callback(*gameEngine);
}

uint32_t GetEquippedItemID(void* equipment, EquipLocation slot)
{
    typedef uint32_t (__thiscall* GetEquippedItemIDProto)(void*, EquipLocation);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetEquippedItemIDProto callback = (GetEquippedItemIDProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_EQUIPPED_ITEM);
    if ((!callback) || (!equipment))
        return 0;

    return callback(equipment, slot);
}

bool IsItemEquipped(void* equipment, uint32_t itemID)
{
    typedef bool (__thiscall* IsItemEquippedProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    IsItemEquippedProto callback = (IsItemEquippedProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_IS_ITEM_EQUIPPED);
    if ((!callback) || (!equipment))
        return 0;

    return callback(equipment, itemID);
}

void ClearPlayerReagents()
{
    typedef void (__thiscall* GetItemsInTabProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    GetItemsInTabProto callback = (GetItemsInTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_CLEAR_PLAYER_REAGENTS);
    void** gameEngine = GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return;

    callback(*gameEngine);
}

}