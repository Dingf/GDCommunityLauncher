#include <unordered_map>
#include "CraftingTemplate.h"

struct ItemTypePairHash
{
    uint64_t operator()(const ItemTypePair& pair)
    {
        return (((uint64_t)pair._itemType & 0xFFFFFFFF) << 32) | ((uint64_t)pair._weaponType & 0xFFFFFFFF);
    }
};

const std::unordered_map<uint64_t, CraftingTemplate> templates =
{
    { ItemTypePair { ITEM_TYPE_HEAD,      WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearhead/tdyn_headlight_a06.dbr",       0.75f, 1.09f } },
    { ItemTypePair { ITEM_TYPE_AMULET,    WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearaccessories/tdyn_necklace_a04.dbr", 0.79f, 1.62f } },
    { ItemTypePair { ITEM_TYPE_CHEST,     WEAPON_TYPE_DEFAULT },   { "records/items/loottables/geartorso/tdyn_torsolight_a05.dbr",     1.22f, 1.32f } },
    { ItemTypePair { ITEM_TYPE_LEGS,      WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearlegs/tdyn_legs_a05.dbr",            1.22f, 0.86f } },
    { ItemTypePair { ITEM_TYPE_FEET,      WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearfeet/tdyn_feet_a05.dbr",            1.22f, 1.08f } },
    { ItemTypePair { ITEM_TYPE_RING,      WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearaccessories/tdyn_ring_a04.dbr",     2.18f, 1.53f } },
    { ItemTypePair { ITEM_TYPE_HANDS,     WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearhands/tdyn_hands_a05.dbr",          1.22f, 1.44f } },
    { ItemTypePair { ITEM_TYPE_BELT,      WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearaccessories/tdyn_waist_a05.dbr",    1.63f, 1.16f } },
    { ItemTypePair { ITEM_TYPE_SHOULDERS, WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearshoulders/tdyn_shoulders_a05.dbr",  1.22f, 1.32f } },
    { ItemTypePair { ITEM_TYPE_MEDAL,     WEAPON_TYPE_DEFAULT },   { "records/items/loottables/gearaccessories/tdyn_medal_a04.dbr",    1.55f, 0.96f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SWORD_1H },  { "records/items/loottables/weapons/tdyn_sword1h_a07.dbr",          0.51f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SWORD_2H },  { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_AXE_1H },    { "records/items/loottables/weapons/tdyn_axe1h_a07.dbr",            0.51f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_AXE_2H },    { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_MACE_1H },   { "records/items/loottables/weapons/tdyn_blunt1h_a07.dbr",          0.51f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_MACE_2H },   { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_DAGGER },    { "records/items/loottables/weapons/tdyn_caster1h_a06.dbr",         0.96f, 1.50f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SCEPTER },   { "records/items/loottables/weapons/tdyn_caster1h_a06.dbr",         0.96f, 1.50f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_RANGED_1H }, { "records/items/loottables/weapons/tdyn_gun1h_a07.dbr",            0.48f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_WEAPON,    WEAPON_TYPE_RANGED_2H }, { "records/items/loottables/weapons/tdyn_gun2h_a07.dbr",            1.58f, 0.43f } },
    { ItemTypePair { ITEM_TYPE_OFFHAND,   WEAPON_TYPE_SHIELD },    { "records/items/loottables/weapons/tdyn_shield_a06.dbr",           1.52f, 0.86f } },
    { ItemTypePair { ITEM_TYPE_OFFHAND,   WEAPON_TYPE_CASTER_OH }, { "records/items/loottables/weapons/tdyn_focus_a05.dbr",            1.81f, 2.18f } },
};

const std::unordered_map<uint64_t, CraftingTemplate>& CraftingTemplate::GetTemplates()
{
    return templates;
}