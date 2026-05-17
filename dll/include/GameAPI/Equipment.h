#ifndef INC_GDCL_DLL_GAME_API_EQUIPMENT_H
#define INC_GDCL_DLL_GAME_API_EQUIPMENT_H

#include <stdint.h>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_PLAYER_EQUIPMENT[] = "?GetEquipmentCtrl@ControllerCharacter@GAME@@QEBAAEBVEquipmentCtrl@2@XZ";
constexpr char GAPI_NAME_GET_EQUIPPED_ITEM[] = "?GetItemId@EquipmentCtrl@GAME@@QEBAIW4EquipmentCtrlLocation@2@@Z";
constexpr char GAPI_NAME_IS_ITEM_EQUIPPED[] = "?IsItemAttached@EquipmentCtrl@GAME@@QEBA_NI@Z";
#else
constexpr char GAPI_NAME_GET_PLAYER_EQUIPMENT[] = "?GetEquipmentCtrl@ControllerCharacter@GAME@@QBEABVEquipmentCtrl@2@XZ";
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

void* GetPlayerEquipment(void* player);
uint32_t GetEquippedItemID(void* equipment, EquipLocation slot);
bool IsItemEquipped(void* equipment, uint32_t itemID);

}

#endif//INC_GDCL_DLL_GAME_API_EQUIPMENT_H