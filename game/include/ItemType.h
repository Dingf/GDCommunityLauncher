#ifndef INC_GDCL_GAME_ITEM_TYPE_H
#define INC_GDCL_GAME_ITEM_TYPE_H

#include <stdint.h>

enum ItemType : uint32_t
{
    ITEM_TYPE_DEFAULT = 0,
    ITEM_TYPE_HEAD = 1,
    ITEM_TYPE_AMULET = 2,
    ITEM_TYPE_CHEST = 3,
    ITEM_TYPE_LEGS = 4,
    ITEM_TYPE_FEET = 5,
    ITEM_TYPE_RING = 6,
    ITEM_TYPE_HANDS = 7,
    ITEM_TYPE_OFFHAND = 8,  // Subtypes are defined in the WeaponType enum
    ITEM_TYPE_WEAPON = 9,   // Subtypes are defined in the WeaponType enum
    //ITEM_TYPE_UNKNOWN = 10,
    ITEM_TYPE_RELIC = 11,
    ITEM_TYPE_FORMULA = 12,
    ITEM_TYPE_BELT = 13,
    ITEM_TYPE_SHOULDERS = 14,
    ITEM_TYPE_MEDAL = 15,
    ITEM_TYPE_COMPONENT = 16,
    ITEM_TYPE_CONSUMABLE = 17,
    ITEM_TYPE_CONSUMABLE_CHEST = 18,
    ITEM_TYPE_QUEST_ITEM = 19,
    ITEM_TYPE_LORE_NOTE = 20,
    ITEM_TYPE_AUGMENT = 21,
    ITEM_TYPE_TRANSMUTER = 22,
    ITEM_TYPE_FACTION_BOOSTER = 23,
    ITEM_TYPE_DEVOTION_RESET = 24,
    ITEM_TYPE_DIFFICULTY_UNLOCK = 25,
    ITEM_TYPE_ATTRIBUTE_RESET = 26,
    MAX_ITEM_TYPES = 27,
};

enum WeaponType : uint32_t
{
    WEAPON_TYPE_DEFAULT = 0,
    //WEAPON_TYPE_UNKNOWN = 1,
    WEAPON_TYPE_AXE_1H = 2,
    WEAPON_TYPE_SWORD_1H = 3,
    WEAPON_TYPE_MACE_1H = 4,
    WEAPON_TYPE_DAGGER = 5,
    WEAPON_TYPE_SCEPTER = 6,
    //WEAPON_TYPE_SPEAR = 7,    // This is the one used in TQ; GD spears use WeaponType = 18 instead
    WEAPON_TYPE_RANGED_2H = 8,
    WEAPON_TYPE_STAFF = 9,
    //WEAPON_TYPE_UNKNOWN = 10,
    WEAPON_TYPE_SHIELD = 11,
    WEAPON_TYPE_CASTER_OH = 12,
    WEAPON_TYPE_RANGED_1H = 13,
    //WEAPON_TYPE_UNKNOWN = 14,
    WEAPON_TYPE_AXE_2H = 15,
    WEAPON_TYPE_SWORD_2H = 16,
    WEAPON_TYPE_MACE_2H = 17,
    WEAPON_TYPE_SPEAR = 18,
};

struct ItemTypePair
{
    ItemType   _itemType;
    WeaponType _weaponType;

    bool operator==(const ItemTypePair& rhs) const
    {
        return (_itemType == rhs._itemType) && (_weaponType == rhs._weaponType);
    }

    operator uint64_t() const
    {
        return (((uint64_t)_itemType & 0xFFFFFFFF) << 32) | ((uint64_t)_weaponType & 0xFFFFFFFF);
    }
};

#endif//INC_GDCL_GAME_ITEM_TYPE_H