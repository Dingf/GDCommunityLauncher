#include <unordered_map>
#include "CraftingTemplate.h"

const std::unordered_map<ItemType, CraftingTemplate> templates =
{
    { ITEM_TYPE_HEAD,      { "records/items/loottables/gearhead/tdyn_headlight_a06.dbr",       0.75f, 1.09f } },
    { ITEM_TYPE_AMULET,    { "records/items/loottables/gearaccessories/tdyn_necklace_a04.dbr", 0.79f, 1.62f } },
    { ITEM_TYPE_CHEST,     { "records/items/loottables/geartorso/tdyn_torsolight_a05.dbr",     1.22f, 1.32f } },
    { ITEM_TYPE_LEGS,      { "records/items/loottables/gearlegs/tdyn_legs_a05.dbr",            1.22f, 0.86f } },
    { ITEM_TYPE_FEET,      { "records/items/loottables/gearfeet/tdyn_feet_a05.dbr",            1.22f, 1.08f } },
    { ITEM_TYPE_RING,      { "records/items/loottables/gearaccessories/tdyn_ring_a04.dbr",     2.18f, 1.53f } },
    { ITEM_TYPE_HANDS,     { "records/items/loottables/gearhands/tdyn_hands_a05.dbr",          1.22f, 1.44f } },
    { ITEM_TYPE_BELT,      { "records/items/loottables/gearaccessories/tdyn_waist_a05.dbr",    1.63f, 1.16f } },
    { ITEM_TYPE_SHOULDERS, { "records/items/loottables/gearshoulders/tdyn_shoulders_a05.dbr",  1.22f, 1.32f } },
    { ITEM_TYPE_MEDAL,     { "records/items/loottables/gearaccessories/tdyn_medal_a04.dbr",    1.55f, 0.96f } },
    { ITEM_TYPE_SWORD_1H,  { "records/items/loottables/weapons/tdyn_sword1h_a07.dbr",          0.51f, 0.43f } },
    { ITEM_TYPE_SWORD_2H,  { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ITEM_TYPE_AXE_1H,    { "records/items/loottables/weapons/tdyn_axe1h_a07.dbr",            0.51f, 0.43f } },
    { ITEM_TYPE_AXE_2H,    { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ITEM_TYPE_MACE_1H,   { "records/items/loottables/weapons/tdyn_blunt1h_a07.dbr",          0.51f, 0.43f } },
    { ITEM_TYPE_MACE_2H,   { "records/items/loottables/weapons/tdyn_melee2h_a07.dbr",          0.82f, 0.43f } },
    { ITEM_TYPE_DAGGER,    { "records/items/loottables/weapons/tdyn_caster1h_a06.dbr",         0.96f, 1.50f } },
    { ITEM_TYPE_SCEPTER,   { "records/items/loottables/weapons/tdyn_caster1h_a06.dbr",         0.96f, 1.50f } },
    { ITEM_TYPE_RANGED_1H, { "records/items/loottables/weapons/tdyn_gun1h_a07.dbr",            0.48f, 0.43f } },
    { ITEM_TYPE_RANGED_2H, { "records/items/loottables/weapons/tdyn_gun2h_a07.dbr",            1.58f, 0.43f } },
    { ITEM_TYPE_SHIELD,    { "records/items/loottables/weapons/tdyn_shield_a06.dbr",           1.52f, 0.86f } },
    { ITEM_TYPE_OFFHAND,   { "records/items/loottables/weapons/tdyn_focus_a05.dbr",            1.81f, 2.18f } },
    { ITEM_TYPE_RELIC,     { "",                                                               0.00f, 0.00f } },
};

const CraftingTemplate& CraftingTemplate::GetTemplate(ItemType type)
{
    return templates.at(type);
}