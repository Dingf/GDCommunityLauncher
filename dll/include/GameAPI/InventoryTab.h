#ifndef INC_GDCL_DLL_GAME_API_INVENTORY_TAB_H
#define INC_GDCL_DLL_GAME_API_INVENTORY_TAB_H

#include <stdint.h>
#include <map>
#include <vector>
#include "EngineAPI/Rect.h"
#include "EngineAPI/Vector.h"

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_TAB_ADD_ITEM_1[] = "?AddItem@InventorySack@GAME@@QEAA_NAEBVVec2@2@PEAVItem@2@_N@Z";
constexpr char GAPI_NAME_TAB_ADD_ITEM_2[] = "?AddItem@InventorySack@GAME@@QEAA_NPEAVItem@2@_N1@Z";
constexpr char GAPI_NAME_TAB_REMOVE_ITEM[] = "?RemoveItem@InventorySack@GAME@@QEAA_NI@Z";
constexpr char GAPI_NAME_TAB_REMOVE_ALL_ITEMS[] = "?RemoveAllItems@InventorySack@GAME@@QEAAXXZ";
constexpr char GAPI_NAME_TAB_GET_ITEMS[] = "?GetInventory@InventorySack@GAME@@QEBAAEBV?$map@IVRect@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_TRANSFER_ADD_ITEM_1[] = "?AddItemToTransfer@GameEngine@GAME@@QEAA_NIAEBVVec2@2@I_N@Z";
constexpr char GAPI_NAME_TRANSFER_ADD_ITEM_2[] = "?AddItemToTransfer@GameEngine@GAME@@QEAA_NII_N@Z";
constexpr char GAPI_NAME_GET_PLAYER_INVENTORY[] = "?GetInventoryCtrl@ControllerPlayer@GAME@@QEBAAEBVPlayerInventoryCtrl@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_EQUIPMENT[] = "?GetEquipmentCtrl@ControllerCharacter@GAME@@QEBAAEBVEquipmentCtrl@2@XZ";
constexpr char GAPI_NAME_GET_INVENTORY_TAB[] = "?GetSack@PlayerInventoryCtrl@GAME@@QEAAPEAVInventorySack@2@H@Z";
constexpr char GAPI_NAME_GET_INVENTORY_TAB_COUNT[] = "?GetNumberOfSacks@PlayerInventoryCtrl@GAME@@QEBAIXZ";
constexpr char GAPI_NAME_GET_PERSONAL_TABS[] = "?GetPrivateStash@Player@GAME@@QEAAAEAV?$vector@PEAVInventorySack@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_GET_TRANSFER_TABS[] = "?GetPlayerTransfer@GameEngine@GAME@@QEAAAEAV?$vector@PEAVInventorySack@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_GET_EQUIPPED_ITEM[] = "?GetItemId@EquipmentCtrl@GAME@@QEBAIW4EquipmentCtrlLocation@2@@Z";
constexpr char GAPI_NAME_IS_ITEM_EQUIPPED[] = "?IsItemAttached@EquipmentCtrl@GAME@@QEBA_NI@Z";
#else
constexpr char GAPI_NAME_TAB_ADD_ITEM_1[] = "?AddItem@InventorySack@GAME@@QAE_NABVVec2@2@PAVItem@2@_N@Z";
constexpr char GAPI_NAME_TAB_ADD_ITEM_2[] = "?AddItem@InventorySack@GAME@@QAE_NPAVItem@2@_N1@Z";
constexpr char GAPI_NAME_TAB_REMOVE_ITEM[] = "?RemoveItem@InventorySack@GAME@@QAE_NI@Z";
constexpr char GAPI_NAME_TAB_REMOVE_ALL_ITEMS[] = "?RemoveAllItems@InventorySack@GAME@@QAEXXZ";
constexpr char GAPI_NAME_TAB_GET_ITEMS[] = "?GetInventory@InventorySack@GAME@@QBEABV?$map@IVRect@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_TRANSFER_ADD_ITEM_1[] = "?AddItemToTransfer@GameEngine@GAME@@QAE_NIABVVec2@2@I_N@Z";
constexpr char GAPI_NAME_TRANSFER_ADD_ITEM_2[] = "?AddItemToTransfer@GameEngine@GAME@@QAE_NII_N@Z";
constexpr char GAPI_NAME_GET_PLAYER_INVENTORY[] = "?GetInventoryCtrl@ControllerPlayer@GAME@@QBEABVPlayerInventoryCtrl@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_EQUIPMENT[] = "?GetEquipmentCtrl@ControllerCharacter@GAME@@QBEABVEquipmentCtrl@2@XZ";
constexpr char GAPI_NAME_GET_INVENTORY_TAB[] = "?GetSack@PlayerInventoryCtrl@GAME@@QAEPAVInventorySack@2@H@Z";
constexpr char GAPI_NAME_GET_INVENTORY_TAB_COUNT[] = "?GetNumberOfSacks@PlayerInventoryCtrl@GAME@@QBEIXZ";
constexpr char GAPI_NAME_GET_PERSONAL_TABS[] = "?GetPrivateStash@Player@GAME@@QAEAAV?$vector@PAVInventorySack@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_GET_TRANSFER_TABS[] = "?GetPlayerTransfer@GameEngine@GAME@@QAEAAV?$vector@PAVInventorySack@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_GET_EQUIPPED_ITEM[] = "?GetItemId@EquipmentCtrl@GAME@@QBEIW4EquipmentCtrlLocation@2@@Z";
constexpr char GAPI_NAME_IS_ITEM_EQUIPPED[] = "?IsItemAttached@EquipmentCtrl@GAME@@QBE_NI@Z";
#endif

enum EquipLocation
{
    EQUIP_LOCATION_HEAD = 1,
    EQUIP_LOCATION_NECK = 2,
    EQUIP_LOCATION_CHEST = 3,
    EQUIP_LOCATION_LEGS = 4,
    EQUIP_LOCATION_FEET = 5,
    EQUIP_LOCATION_RING1 = 6,
    EQUIP_LOCATION_RING2 = 7,
    EQUIP_LOCATION_HANDS = 8,
    EQUIP_LOCATION_MAIN_HAND = 9,
    EQUIP_LOCATION_OFFHAND = 10,
    EQUIP_LOCATION_RELIC = 11,
    EQUIP_LOCATION_WAIST = 12,
    EQUIP_LOCATION_SHOULDER = 13,
    EQUIp_LOCATION_MEDAL = 14
};

bool AddItemToTab(void* tab, const EngineAPI::Vec2& position, void* item, bool unk1);
bool AddItemToTab(void* tab, void* item, bool unk1, bool unk2);
bool RemoveItemFromTab(void* tab, uint32_t itemID);
void RemoveAllItemsFromTab(void* tab);
const std::map<uint32_t, EngineAPI::Rect>& GetItemsInTab(void* tab);
bool AddItemToTransfer(uint32_t tab, const EngineAPI::Vec2& position, uint32_t itemID, bool unk1);
bool AddItemToTransfer(uint32_t itemID, uint32_t tab, bool unk1);
void* GetPlayerInventory(void* player);
void* GetPlayerEquipment(void* player);
void* GetInventoryTab(void* inventory, int32_t index);
uint32_t GetInventoryTabCount(void* inventory);
const std::vector<void*>& GetPersonalTabs(void* player);
const std::vector<void*>& GetTransferTabs();
uint32_t GetEquippedItemID(void* equipment, EquipLocation slot);
bool IsItemEquipped(void* equipment, uint32_t itemID);

}

#endif//INC_GDCL_DLL_GAME_API_INVENTORY_TAB_H