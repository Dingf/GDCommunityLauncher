#ifndef INC_GDCL_DLL_GAME_API_ITEM_H
#define INC_GDCL_DLL_GAME_API_ITEM_H

#include <stdint.h>
#include <string>
#include "ItemType.h"

struct Item;
namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_CREATE_ITEM[] = "?CreateItem@Item@GAME@@SAPEAV12@AEBUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_GET_ITEM_REPLICA_INFO[] = "?GetItemReplicaInfo@Item@GAME@@UEBAXAEAUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_SET_ITEM_REPLICA_INFO[] = "?SetItemReplicaInfo@Item@GAME@@QEAAXAEBUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_SET_ITEM_VISIBLE_PLAYER[] = "?SetVisiblePlayer@Item@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_GET_ITEM_LEVEL[] = "?GetItemLevel@Item@GAME@@QEBAIXZ";
constexpr char GAPI_NAME_USE_ITEM_ENCHANTMENT[] = "?UseOn@ItemEnchantment@GAME@@UEAA_NPEAVEntity@2@_NAEA_N@Z";
constexpr char GAPI_NAME_CAN_ENCHANT_BE_USED_ON[] = "?CanBeUsedOn@ItemEnchantment@GAME@@UEBA_NPEAVEntity@2@_NAEA_N@Z";
constexpr char GAPI_NAME_GET_ITEM_DESCRIPTION[] = "?GetUIGameDescription@Item@GAME@@UEBAXAEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char GAPI_NAME_GET_WEAPON_DESCRIPTION[] = "?GetUIGameDescription@Weapon@GAME@@UEBAXAEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char GAPI_NAME_GET_ARMOR_DESCRIPTION[] = "?GetUIGameDescription@Armor@GAME@@UEBAXAEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char GAPI_NAME_GET_ITEM_CLASSIFICATION[] = "?GetItemClassification@Item@GAME@@UEBA?AW4ItemClassification@2@_N@Z";
constexpr char GAPI_NAME_GET_ITEM_BITMAP[] = "?GetBitmap@Item@GAME@@UEBAPEBVGraphicsTexture@2@XZ";
#else
constexpr char GAPI_NAME_CREATE_ITEM[] = "?CreateItem@Item@GAME@@SAPAV12@ABUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_GET_ITEM_REPLICA_INFO[] = "?GetItemReplicaInfo@Item@GAME@@UBEXAAUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_SET_ITEM_REPLICA_INFO[] = "?SetItemReplicaInfo@Item@GAME@@QAEXABUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_SET_ITEM_VISIBLE_PLAYER[] = "?SetVisiblePlayer@Item@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_GET_ITEM_LEVEL[] = "?GetItemLevel@Item@GAME@@QBEIXZ";
constexpr char GAPI_NAME_USE_ITEM_ENCHANTMENT[] = "?UseOn@ItemEnchantment@GAME@@UAE_NPAVEntity@2@_NAA_N@Z";
constexpr char GAPI_NAME_CAN_ENCHANT_BE_USED_ON[] = "?CanBeUsedOn@ItemEnchantment@GAME@@UBE_NPAVEntity@2@_NAA_N@Z";
constexpr char GAPI_NAME_GET_ITEM_DESCRIPTION[] = "?GetUIGameDescription@Item@GAME@@UBEXAAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char GAPI_NAME_GET_ITEM_CLASSIFICATION[] = "?GetItemClassification@Item@GAME@@UBE?AW4ItemClassification@2@_N@Z";
constexpr char GAPI_NAME_GET_ITEM_BITMAP[] = "?GetBitmap@Item@GAME@@UBEPBVGraphicsTexture@2@XZ";
#endif

enum ItemClassification : uint32_t
{
    ITEM_CLASSIFICATION_NORMAL = 0,
    ITEM_CLASSIFICATION_MAGIC = 1,
    ITEM_CLASSIFICATION_RARE = 2,
    ITEM_CLASSIFICATION_EPIC = 3,
    ITEM_CLASSIFICATION_LEGEND = 4,
    ITEM_CLASSIFICATION_BROKEN = 5,
    ITEM_CLASSIFICATION_POTION = 6,
    //ITEM_CLASSIFICATION_UNKNOWN = 7
    ITEM_CLASSIFICATION_RELIC = 8,
    ITEM_CLASSIFICATION_QUEST = 9,
    ITEM_CLASSIFICATION_ARTIFACT = 10,
    ITEM_CLASSIFICATION_FORMULA = 11,
    //ITEM_CLASSIFICATION_UNKNOWN = 12
    ITEM_CLASSIFICATION_LORE_NOTE = 13,
};

struct ItemReplicaInfo
{
    ItemReplicaInfo() {}

    uint32_t    _itemID;
    std::string _itemName;
    std::string _itemPrefix;
    std::string _itemSuffix;
    uint32_t    _itemSeed;
    uint32_t    _unk1;
    std::string _itemModifier;
    std::string _itemComponent;
    std::string _itemCompletion;
    uint32_t    _itemComponentSeed;
    uint32_t    _unk2;
    std::string _itemAugment;
    uint32_t    _unk3;      // _itemUnk1
    uint32_t    _itemAugmentSeed;
    std::string _itemIllusion;
    uint32_t    _unk4;      // _itemUnk2
    uint32_t    _unk5;
    uint64_t    _unk6;
    uint32_t    _unk7;
    uint32_t    _itemStackCount;
    uint32_t    _unk8;
    uint32_t    _unk9;
    uint32_t    _unk10;
    uint32_t    _unk11;
};

void* CreateItem(const ItemReplicaInfo& info);
uint32_t GenerateItemSeed(uint32_t max = 0xFFFFFFFF);
ItemReplicaInfo GetItemReplicaInfo(void* item);
void SetItemReplicaInfo(void* item, const ItemReplicaInfo& info);
void SetItemVisiblePlayer(void* item, uint32_t playerID);
uint32_t GetItemLevel(void* item);
ItemClassification GetItemClassification(void* item);
void* GetItemBitmap(void* item);
uint32_t GetItemWidth(void* item);
uint32_t GetItemHeight(void* item);
ItemType GetItemType(void* item);
WeaponType GetWeaponType(void* item);
std::string GetItemNameTag(void* item);
std::string GetItemPrefixTag(void* item);
std::string GetItemSuffixTag(void* item);
ItemReplicaInfo ItemToInfo(const Item& item);
Item InfoToItem(const GameAPI::ItemReplicaInfo& info);

}

#endif//INC_GDCL_DLL_GAME_API_ITEM_H