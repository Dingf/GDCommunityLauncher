#include <Windows.h>
#include "GameAPI/GameEngine.h"
#include "GameAPI/InventoryTab.h"
#include "GameAPI/Player.h"

namespace GameAPI
{

std::map<uint32_t, EngineAPI::Rect> defaultItemMap;
std::vector<void*> defaultTabs;

bool AddItemToTab(void* tab, const EngineAPI::Vec2& position, void* item, bool unk1)
{
    typedef bool(__thiscall* AddItemToTabProto)(void*, const EngineAPI::Vec2&, void*, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    AddItemToTabProto callback = (AddItemToTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_ADD_ITEM);
    if ((!callback) || (!tab))
        return false;

    return callback(tab, position, item, unk1);
}

bool RemoveItemFromTab(void* tab, uint32_t itemID)
{
    typedef bool(__thiscall* RemoveItemFromTabProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    RemoveItemFromTabProto callback = (RemoveItemFromTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_REMOVE_ITEM);
    if ((!callback) || (!tab))
        return false;

    return callback(tab, itemID);
}

const std::map<uint32_t, EngineAPI::Rect>& GetItemsInTab(void* tab)
{
    typedef const std::map<uint32_t, EngineAPI::Rect>& (__thiscall* GetItemsInTabProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return defaultItemMap;

    GetItemsInTabProto callback = (GetItemsInTabProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAB_GET_ITEMS);
    if ((!callback) || (!tab))
        return defaultItemMap;

    return callback(tab);
}

void* GetPlayerInventory(void* player)
{
    typedef void* (__thiscall* GetPlayerInventoryProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return 0;

    GetInventoryTabCountProto callback = (GetInventoryTabCountProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_INVENTORY_TAB_COUNT);
    if ((!callback) || (!inventory))
        return 0;

    return callback(inventory);
}

const std::vector<void*>& GetPersonalTabs(void* player)
{
    typedef const std::vector<void*>& (__thiscall* GetPersonalTabsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return defaultTabs;

    GetPersonalTabsProto callback = (GetPersonalTabsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PERSONAL_TABS);
    if ((!callback) || (!player))
        return defaultTabs;

    return callback(player);
}

const std::vector<void*>& GetTransferTabs()
{
    typedef const std::vector<void*>& (__thiscall* GetTransferTabsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return defaultTabs;

    GetTransferTabsProto callback = (GetTransferTabsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_TRANSFER_TABS);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return defaultTabs;

    return callback(*gameEngine);
}

bool IsItemEquipped(void* equipment, uint32_t itemID)
{
    typedef bool(__thiscall* IsItemEquippedProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return 0;

    IsItemEquippedProto callback = (IsItemEquippedProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_IS_ITEM_EQUIPPED);
    if ((!callback) || (!equipment))
        return 0;

    return callback(equipment, itemID);
}

}