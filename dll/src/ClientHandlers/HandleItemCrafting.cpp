#include <unordered_map>
#include <random>
#include <regex>
#include "ClientHandler.h"
#include "CraftingDatabase.h"

struct ImprintFlags
{
    uint32_t _itemTypeFlags;
    uint32_t _weaponTypeFlags;

    bool Matches(ItemType itemType, WeaponType weaponType) const
    {
        if ((_itemTypeFlags & (1 << itemType)) != 0)
        {
            return ((_weaponTypeFlags == 0) ||
                   ((_weaponTypeFlags & (1 << weaponType)) != 0));
        }
        return false;
    }
};

std::unordered_map<std::string, ImprintFlags> _imprintTypeMap =
{
    { "grimleague/items/enchants/r201_imprint_helm.dbr",      { (1 << ITEM_TYPE_HEAD),      0 } },
    { "grimleague/items/enchants/r202_imprint_torso.dbr",     { (1 << ITEM_TYPE_CHEST),     0 } },
    { "grimleague/items/enchants/r203_imprint_gloves.dbr",    { (1 << ITEM_TYPE_HANDS),     0 } },
    { "grimleague/items/enchants/r204_imprint_shoulders.dbr", { (1 << ITEM_TYPE_SHOULDERS), 0 } },
    { "grimleague/items/enchants/r205_imprint_pants.dbr",     { (1 << ITEM_TYPE_LEGS),      0 } },
    { "grimleague/items/enchants/r206_imprint_boots.dbr",     { (1 << ITEM_TYPE_FEET),      0 } },
    { "grimleague/items/enchants/r207_imprint_belt.dbr",      { (1 << ITEM_TYPE_BELT),      0 } },
    { "grimleague/items/enchants/r208_imprint_medal.dbr",     { (1 << ITEM_TYPE_MEDAL),     0 } },
    { "grimleague/items/enchants/r209_imprint_ring.dbr",      { (1 << ITEM_TYPE_RING),      0 } },
    { "grimleague/items/enchants/r210_imprint_amulet.dbr",    { (1 << ITEM_TYPE_AMULET),    0 } },
    { "grimleague/items/enchants/r211_imprint_1h.dbr",        { (1 << ITEM_TYPE_WEAPON),    (1 << WEAPON_TYPE_SWORD_1H) | (1 << WEAPON_TYPE_AXE_1H) | (1 << WEAPON_TYPE_MACE_1H) | (1 << WEAPON_TYPE_RANGED_1H) } },
    { "grimleague/items/enchants/r212_imprint_1hcaster.dbr",  { (1 << ITEM_TYPE_WEAPON),    (1 << WEAPON_TYPE_DAGGER)   | (1 << WEAPON_TYPE_SCEPTER) } },
    { "grimleague/items/enchants/r211_imprint_2h.dbr",        { (1 << ITEM_TYPE_WEAPON),    (1 << WEAPON_TYPE_SWORD_2H) | (1 << WEAPON_TYPE_AXE_2H) | (1 << WEAPON_TYPE_MACE_2H) | (1 << WEAPON_TYPE_RANGED_2H) } },
    { "grimleague/items/enchants/r214_imprint_shield.dbr",    { (1 << ITEM_TYPE_OFFHAND),   (1 << WEAPON_TYPE_SHIELD) } },
    { "grimleague/items/enchants/r215_imprint_offhand.dbr",   { (1 << ITEM_TYPE_OFFHAND),   (1 << WEAPON_TYPE_CASTER_OH) } },
};

const std::regex vaalRegex("^grimleague/items/lootaffixes/ultos/ultos_affix(\\d{2}[a-z]).dbr$");
const std::regex smithRegex("^grimleague/items/lootaffixes/ultos/ultos_smith(\\d{2}[a-z]).dbr$");

void* _lastEnchantUsedOn = nullptr;
void* _lastEquipment = nullptr;
void* _lastEnchant = nullptr;
GameAPI::ItemReplicaInfo _lastEnchantInfo;

std::string GenerateAffixRoll(const std::string& tableName, uint32_t itemLevel, std::string ignore)
{
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();
    if (craftingDB.HasTable(tableName))
    {
        CraftingDatabase::CraftingDBTable table = craftingDB.GetTable(tableName);

        uint32_t totalWeight = 0;
        for (const auto& pair : table)
        {
            const CraftingDatabase::CraftingDBEntry& entry = pair.second;
            if ((itemLevel >= entry._minLevel) && (itemLevel <= entry._maxLevel) && (ignore != entry._name))
                totalWeight += entry._weight;
        }

        int64_t rollWeight = GameAPI::GenerateItemSeed(totalWeight);
        for (const auto& pair : table)
        {
            const CraftingDatabase::CraftingDBEntry& entry = pair.second;
            if ((itemLevel >= entry._minLevel) && (itemLevel <= entry._maxLevel) && (ignore != entry._name))
            {
                rollWeight -= entry._weight;
                if (rollWeight <= 0)
                    return entry._name;
            }
        }
    }
    return {};
}

bool IsWeaponType(ItemType itemType)
{
    return ((itemType == ITEM_TYPE_OFFHAND) || (itemType == ITEM_TYPE_WEAPON));
}

std::string GenerateItemPrefix(ItemType itemType, WeaponType weaponType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "prefix" + std::to_string(itemType) + "_";
    if (IsWeaponType(itemType))
        tableName += std::to_string(weaponType);
    else
        tableName += "0";

    return GenerateAffixRoll(tableName, itemLevel, ignore);
}

std::string GenerateItemSuffix(ItemType itemType, WeaponType weaponType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "suffix" + std::to_string(itemType) + "_";
    if (IsWeaponType(itemType))
        tableName += std::to_string(weaponType);
    else
        tableName += "0";

    return GenerateAffixRoll(tableName, itemLevel, ignore);
}

std::string GenerateVaalAffix(ItemType itemType, WeaponType weaponType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "vaal" + std::to_string(itemType) + "_";
    if (IsWeaponType(itemType))
        tableName += std::to_string(weaponType);
    else
        tableName += "0";

    return GenerateAffixRoll(tableName, itemLevel, ignore);
}

std::string GenerateSmithAffix(ItemType itemType, WeaponType weaponType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "smith" + std::to_string(itemType) + "_";
    if (IsWeaponType(itemType))
        tableName += std::to_string(weaponType);
    else
        tableName += "0";

    return GenerateAffixRoll(tableName, itemLevel, ignore);
}

std::string UpgradeItemBase(const std::string& itemName)
{
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "upgrade";
    if (craftingDB.HasTable(tableName))
    {
        CraftingDatabase::CraftingDBTable table = craftingDB.GetTable(tableName);
        if (table.count(itemName) > 0)
        {
            return table.at(itemName)._value;
        }
    }
    return {};
}

std::string DowngradeItemBase(const std::string& itemName)
{
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "downgrade";
    if (craftingDB.HasTable(tableName))
    {
        CraftingDatabase::CraftingDBTable table = craftingDB.GetTable(tableName);
        if (table.count(itemName) > 0)
        {
            return table.at(itemName)._value;
        }
    }
    return {};
}

inline bool HasVaalAffix(const GameAPI::ItemReplicaInfo& itemInfo)
{
    return (std::regex_match(itemInfo._itemPrefix, vaalRegex) || std::regex_match(itemInfo._itemSuffix, vaalRegex));
}

inline bool HasSmithAffix(const GameAPI::ItemReplicaInfo& itemInfo)
{
    return (std::regex_match(itemInfo._itemModifier, smithRegex) || std::regex_match(itemInfo._itemModifier, smithRegex));
}

inline bool HasVaalOrSmithAffix(const GameAPI::ItemReplicaInfo& itemInfo)
{
    return (HasVaalAffix(itemInfo) || HasSmithAffix(itemInfo));
}

// CanRerollItemSeed()
bool CanRerollItemSeed(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return true;
}

// CanRerollItemAffix() [DEPRECATED]
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item cannot be a relic
bool CanRerollItemAffix(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE)) && (itemType != ITEM_TYPE_RELIC);
}

// CanRerollItemAffixLowLevel() [DEPRECATED]
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item must have an ilvl of 70 or less
//   - Item cannot be a relic
bool CanRerollItemAffixLowLevel(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemLevel <= 70) && (itemType != ITEM_TYPE_RELIC));
}

// CanRerollItemPrefix()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item must have an existing prefix
//   - Item cannot be a relic
bool CanRerollItemPrefix(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemType != ITEM_TYPE_RELIC) && (!itemInfo._itemPrefix.empty()));
}

// CanRerolltemSuffix()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item cannot be a relic
//   - Item must have an existing suffix
bool CanRerollItemSuffix(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemType != ITEM_TYPE_RELIC) && (!itemInfo._itemSuffix.empty()));
}

// CanRerollItemPrefixLowLevel()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item must have an ilvl of 70 or less
//   - Item cannot be a relic
//   - Item must have an existing prefix
bool CanRerollItemPrefixLowLevel(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemLevel <= 70) && (itemType != ITEM_TYPE_RELIC) && (!itemInfo._itemPrefix.empty()));
}

// CanRerollItemSuffixLowLevel()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare or less
//   - Item must have an ilvl of 70 or less
//   - Item cannot be a relic
//   - Item must have an existing suffix
bool CanRerollItemSuffixLowLevel(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemLevel <= 70) && (itemType != ITEM_TYPE_RELIC) && (!itemInfo._itemSuffix.empty()));
}

// CanAddRareAffix()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Rare
//   - Item cannot be a relic
//   - Item must have an empty prefix OR empty suffix
bool CanAddRareAffix(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity == GameAPI::ITEM_CLASSIFICATION_RARE)) && (itemType != ITEM_TYPE_RELIC) && ((itemInfo._itemPrefix.empty() || itemInfo._itemSuffix.empty()));
}

// CanRerollEpicAffix()
//   - Item must not have a vaal affix
//   - Item must be have a rarity of Epic
//   - Item cannot be a relic
bool CanRerollEpicAffix(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity == GameAPI::ITEM_CLASSIFICATION_EPIC)) && (itemType != ITEM_TYPE_RELIC);
}

// CanRerollVaal()
//   - Item must not have a vaal smith affix
//   - Item cannot be a relic
bool CanRerollVaal(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasSmithAffix(itemInfo) && (itemType != ITEM_TYPE_RELIC));
}

// CanRerollSmith()
//   - Item must not have a regular vaal affix
//   - Item cannot be a relic
bool CanRerollSmith(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalAffix(itemInfo) && (itemType != ITEM_TYPE_RELIC));
}

bool CanGenerateItemPrefix(ItemType itemType, uint32_t itemLevel)
{
    std::string tableName = "prefix" + std::to_string(itemType);
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).size() > 0);
    }
    return false;
}

bool CanGenerateItemSuffix(ItemType itemType, uint32_t itemLevel)
{
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();
    std::string tableName = "suffix" + std::to_string(itemType);
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).size() > 0);
    }
    return false;
}

bool CanUpgradeItemBase(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalOrSmithAffix(itemInfo) || (itemType == ITEM_TYPE_RELIC) || (itemRarity >= GameAPI::ITEM_CLASSIFICATION_LEGEND))
        return false;

    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "upgrade";
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).count(itemInfo._itemName) > 0);
    }
    return false;
}

bool CanDowngradeItemBase(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalOrSmithAffix(itemInfo) || (itemType == ITEM_TYPE_RELIC) || (itemRarity >= GameAPI::ITEM_CLASSIFICATION_LEGEND))
        return false;

    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "downgrade";
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).count(itemInfo._itemName) > 0);
    }
    return false;
}

bool CanUpgradeItemBase(void* item, const GameAPI::ItemReplicaInfo& itemInfo)
{
    return (!HasVaalOrSmithAffix(itemInfo) && CanUpgradeItemBase(item, nullptr, 0, itemInfo, ITEM_TYPE_DEFAULT, GameAPI::ITEM_CLASSIFICATION_NORMAL));
}

bool CanDowngradeItemBase(void* item, const GameAPI::ItemReplicaInfo& itemInfo)
{
    return (!HasVaalOrSmithAffix(itemInfo) && CanDowngradeItemBase(item, nullptr, 0, itemInfo, ITEM_TYPE_DEFAULT, GameAPI::ITEM_CLASSIFICATION_NORMAL));
}

bool CanUpgradeItemBaseLowLevel(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (CanUpgradeItemBase(item, itemInfo) && (itemLevel <= 49));
}

bool CanCreateTransferAugment(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalOrSmithAffix(itemInfo) && (itemRarity < GameAPI::ITEM_CLASSIFICATION_LEGEND) && (itemType != ITEM_TYPE_RELIC) && (!itemInfo._itemPrefix.empty() || !itemInfo._itemSuffix.empty()));
}

bool CanUseTransferAugment(void* item, void* enchant, uint32_t itemLevel, const GameAPI::ItemReplicaInfo& itemInfo, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalOrSmithAffix(itemInfo))
        return false;

    GameAPI::ItemReplicaInfo enchantInfo = GameAPI::GetItemReplicaInfo(enchant);

    // Prevent transfers on legendary items and on epic items if the augment has both a prefix and a suffix
    if ((itemType == ITEM_TYPE_RELIC) || (itemRarity >= GameAPI::ITEM_CLASSIFICATION_LEGEND) || ((itemRarity == GameAPI::ITEM_CLASSIFICATION_EPIC) && (!enchantInfo._itemPrefix.empty()) && (!enchantInfo._itemSuffix.empty())))
        return false;

    std::string enchantName = enchantInfo._itemName;
    if (_imprintTypeMap.count(enchantName) > 0)
    {
        WeaponType weaponType = GameAPI::GetWeaponType(item);
        return _imprintTypeMap.at(enchantName).Matches(itemType, weaponType);
    }
    return false;
}

bool HandleRerollItemSeed(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    itemInfo._itemSeed = GameAPI::GenerateItemSeed();
    return true;
}

bool HandleRerollItemPrefix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    ItemType itemType = GameAPI::GetItemType(item);
    WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;
    itemInfo._itemPrefix = GenerateItemPrefix(itemType, weaponType, GameAPI::GetItemLevel(item), itemInfo._itemPrefix);
    return true;
}

bool HandleRerollItemSuffix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    ItemType itemType = GameAPI::GetItemType(item);
    WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;
    itemInfo._itemSuffix = GenerateItemSuffix(itemType, weaponType, GameAPI::GetItemLevel(item), itemInfo._itemSuffix);
    return true;
}

bool HandleRerollRareAffix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    uint32_t itemLevel = GameAPI::GetItemLevel(item);
    ItemType itemType = GameAPI::GetItemType(item);
    WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;

    if (itemInfo._itemPrefix.empty() && itemInfo._itemSuffix.empty())
    {
        uint32_t roll = GameAPI::GenerateItemSeed();
        if ((roll % 2) == 0)
            itemInfo._itemPrefix = GenerateItemPrefix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
        else
            itemInfo._itemSuffix = GenerateItemSuffix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
    }
    else if (itemInfo._itemPrefix.empty())
    {
        itemInfo._itemPrefix = GenerateItemPrefix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
    }
    else if (itemInfo._itemSuffix.empty())
    {
        itemInfo._itemSuffix = GenerateItemSuffix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
    }
    return true;
}

bool HandleRerollEpicAffix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    uint32_t itemLevel = GameAPI::GetItemLevel(item);
    ItemType itemType = GameAPI::GetItemType(item);
    WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;

    uint32_t roll = GameAPI::GenerateItemSeed();
    if ((roll % 2) == 0)
    {
        itemInfo._itemPrefix = GenerateItemPrefix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
        itemInfo._itemSuffix = "";
    }
    else
    {
        itemInfo._itemPrefix = "";
        itemInfo._itemSuffix = GenerateItemSuffix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
    }
    return true;
}

bool HandleUpgradeItemBase(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    if (CanUpgradeItemBase(item, itemInfo))
    {
        itemInfo._itemName = UpgradeItemBase(itemName);
        return true;
    }
    return false;
}

bool HandleDowngradeItemBase(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    if (CanDowngradeItemBase(item, itemInfo))
    {
        itemInfo._itemName = DowngradeItemBase(itemName);
        return true;
    }
    return false;
}

void HandleVaalDestroyItem(void* item, GameAPI::ItemReplicaInfo& itemInfo)
{
    // Brick the item, e.g. turn it into Scrap/Aether Crystal
    itemInfo._itemPrefix = "";
    itemInfo._itemSuffix = "";
    itemInfo._itemModifier = "";
    if ((GameAPI::GetItemWidth(item) >= 2) && (GameAPI::GetItemHeight(item) >= 2))
    {
        itemInfo._itemName = "records/items/questitems/scrapmetal.dbr";
        itemInfo._itemStackCount = (GameAPI::GenerateItemSeed() % 4) + 1;
    }
    else
    {
        itemInfo._itemName = "records/items/materia/compa_aethercrystal.dbr";
        itemInfo._itemStackCount = (GameAPI::GenerateItemSeed() % 4) + 1;
    }
}

bool HandleRerollVaal(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    uint32_t roll = GameAPI::GenerateItemSeed() % 100;
    if (roll < 25)
    {
        HandleVaalDestroyItem(item, itemInfo);
    }
    else
    {
        // Add a special vaal bonus affix
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        ItemType itemType = GameAPI::GetItemType(item);
        WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;

        // Prioritize overwriting existing vaal affixes over empty prefix/suffix slots
        if (std::regex_match(itemInfo._itemPrefix, vaalRegex))
            itemInfo._itemPrefix = GenerateVaalAffix(itemType, weaponType, itemLevel, itemInfo._itemPrefix);
        else if (std::regex_match(itemInfo._itemSuffix, vaalRegex))
            itemInfo._itemSuffix = GenerateVaalAffix(itemType, weaponType, itemLevel, itemInfo._itemSuffix);
        else if (itemInfo._itemPrefix.empty())
            itemInfo._itemPrefix = GenerateVaalAffix(itemType, weaponType, itemLevel, {});
        else
            itemInfo._itemSuffix = GenerateVaalAffix(itemType, weaponType, itemLevel, {});
    }
    return true;
}

bool HandleRerollSmith(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    uint32_t roll = GameAPI::GenerateItemSeed() % 100;
    if (roll < 25)
    {
        HandleVaalDestroyItem(item, itemInfo);
    }
    else
    {
        // Add a special smithing bonus affix
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        ItemType itemType = GameAPI::GetItemType(item);
        WeaponType weaponType = IsWeaponType(itemType) ? GameAPI::GetWeaponType(item) : WEAPON_TYPE_DEFAULT;

        itemInfo._itemModifier = GenerateSmithAffix(itemType, weaponType, itemLevel, itemInfo._itemModifier);
        itemInfo._itemSeed = GameAPI::GenerateItemSeed();
    }
    return true;
}

bool HandleCreateTransferAugment(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    if ((!itemInfo._itemPrefix.empty() || !itemInfo._itemSuffix.empty()))
    {
        ItemType itemType = GameAPI::GetItemType(item);
        WeaponType weaponType = GameAPI::GetWeaponType(item);
        if (((1 << itemType) & 0xE3FE) != 0)
        {
            for (const auto& pair : _imprintTypeMap)
            {
                if (pair.second.Matches(itemType, weaponType))
                {
                    if (!itemInfo._itemPrefix.empty())
                        itemInfo._itemPrefix = itemInfo._itemPrefix + ":" + GameAPI::GetItemPrefixTag(item);
                    if (!itemInfo._itemSuffix.empty())
                        itemInfo._itemSuffix = itemInfo._itemSuffix + ":" + GameAPI::GetItemSuffixTag(item);

                    itemInfo._itemName = pair.first;
                    return true;
                }
            }
        }
    }
    return false;
}

bool HandleUseTransferAugment(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    GameAPI::ItemReplicaInfo enchantInfo = GameAPI::GetItemReplicaInfo(enchant);

    const static std::regex transferRegex("^([^:]*):([^:]*)$");

    std::smatch prefixMatch;
    std::smatch suffixMatch;

    bool hasPrefix = std::regex_match(enchantInfo._itemPrefix, prefixMatch, transferRegex);
    bool hasSuffix = std::regex_match(enchantInfo._itemSuffix, suffixMatch, transferRegex);
    if (hasPrefix || hasSuffix)
    {
        itemInfo._itemPrefix = prefixMatch.str(1);
        itemInfo._itemSuffix = suffixMatch.str(1);
        itemInfo._itemSeed = enchantInfo._itemSeed;
        return true;
    }
    return false;
}

typedef bool (*ItemCraftingHandler)(void*, void*, GameAPI::ItemReplicaInfo&);
std::unordered_map<std::string, ItemCraftingHandler> itemCraftingHandlers =
{
    { "grimleague/items/enchants/r101_pre.dbr",               HandleRerollItemPrefix },
    { "grimleague/items/enchants/r102_suff.dbr",              HandleRerollItemSuffix },
    { "grimleague/items/enchants/r103_pre_70.dbr",            HandleRerollItemPrefix },
    { "grimleague/items/enchants/r104_suff_70.dbr",           HandleRerollItemSuffix },
    { "grimleague/items/enchants/r105_seed.dbr",              HandleRerollItemSeed },
    { "grimleague/items/enchants/r106_upgrade.dbr",           HandleUpgradeItemBase },
    { "grimleague/items/enchants/r106a_upgrade.dbr",          HandleUpgradeItemBase },
    { "grimleague/items/enchants/r107_vaal_rare.dbr",         HandleRerollRareAffix },
    { "grimleague/items/enchants/r107_vaal_epic.dbr",         HandleRerollEpicAffix },
    { "grimleague/items/enchants/r108_vaal_legendary.dbr",    HandleRerollVaal },
    { "grimleague/items/enchants/r108_vaal_smith.dbr",        HandleRerollSmith },
    { "grimleague/items/enchants/r216_empty.dbr",             HandleCreateTransferAugment },
    { "grimleague/items/enchants/r201_imprint_helm.dbr",      HandleUseTransferAugment },
    { "grimleague/items/enchants/r202_imprint_torso.dbr",     HandleUseTransferAugment },
    { "grimleague/items/enchants/r203_imprint_gloves.dbr",    HandleUseTransferAugment },
    { "grimleague/items/enchants/r204_imprint_shoulders.dbr", HandleUseTransferAugment },
    { "grimleague/items/enchants/r205_imprint_pants.dbr",     HandleUseTransferAugment },
    { "grimleague/items/enchants/r206_imprint_boots.dbr",     HandleUseTransferAugment },
    { "grimleague/items/enchants/r207_imprint_belt.dbr",      HandleUseTransferAugment },
    { "grimleague/items/enchants/r208_imprint_medal.dbr",     HandleUseTransferAugment },
    { "grimleague/items/enchants/r209_imprint_ring.dbr",      HandleUseTransferAugment },
    { "grimleague/items/enchants/r210_imprint_amulet.dbr",    HandleUseTransferAugment },
    { "grimleague/items/enchants/r211_imprint_1h.dbr",        HandleUseTransferAugment },
    { "grimleague/items/enchants/r212_imprint_1hcaster.dbr",  HandleUseTransferAugment },
    { "grimleague/items/enchants/r213_imprint_2h.dbr",        HandleUseTransferAugment },
    { "grimleague/items/enchants/r214_imprint_shield.dbr",    HandleUseTransferAugment },
    { "grimleague/items/enchants/r215_imprint_offhand.dbr",   HandleUseTransferAugment },
};

typedef bool (*CanUseItemFilter)(void*, void*, uint32_t, const GameAPI::ItemReplicaInfo&, ItemType, GameAPI::ItemClassification);
std::unordered_map<std::string, CanUseItemFilter> canUseItemFilters =
{
    { "grimleague/items/enchants/r101_pre.dbr",               CanRerollItemPrefix },
    { "grimleague/items/enchants/r102_suff.dbr",              CanRerollItemSuffix },
    { "grimleague/items/enchants/r103_pre_70.dbr",            CanRerollItemPrefixLowLevel },
    { "grimleague/items/enchants/r104_suff_70.dbr",           CanRerollItemSuffixLowLevel },
    { "grimleague/items/enchants/r105_seed.dbr",              CanRerollItemSeed },
    { "grimleague/items/enchants/r106_upgrade.dbr",           CanUpgradeItemBase },
    { "grimleague/items/enchants/r106a_upgrade.dbr",          CanUpgradeItemBaseLowLevel },
    { "grimleague/items/enchants/r107_vaal_rare.dbr",         CanAddRareAffix },
    { "grimleague/items/enchants/r107_vaal_epic.dbr",         CanRerollEpicAffix },
    { "grimleague/items/enchants/r108_vaal_legendary.dbr",    CanRerollVaal },
    { "grimleague/items/enchants/r108_vaal_smith.dbr",        CanRerollSmith },
    { "grimleague/items/enchants/r216_empty.dbr",             CanCreateTransferAugment },
    { "grimleague/items/enchants/r201_imprint_helm.dbr",      CanUseTransferAugment },
    { "grimleague/items/enchants/r202_imprint_torso.dbr",     CanUseTransferAugment },
    { "grimleague/items/enchants/r203_imprint_gloves.dbr",    CanUseTransferAugment },
    { "grimleague/items/enchants/r204_imprint_shoulders.dbr", CanUseTransferAugment },
    { "grimleague/items/enchants/r205_imprint_pants.dbr",     CanUseTransferAugment },
    { "grimleague/items/enchants/r206_imprint_boots.dbr",     CanUseTransferAugment },
    { "grimleague/items/enchants/r207_imprint_belt.dbr",      CanUseTransferAugment },
    { "grimleague/items/enchants/r208_imprint_medal.dbr",     CanUseTransferAugment },
    { "grimleague/items/enchants/r209_imprint_ring.dbr",      CanUseTransferAugment },
    { "grimleague/items/enchants/r210_imprint_amulet.dbr",    CanUseTransferAugment },
    { "grimleague/items/enchants/r211_imprint_1h.dbr",        CanUseTransferAugment },
    { "grimleague/items/enchants/r212_imprint_1hcaster.dbr",  CanUseTransferAugment },
    { "grimleague/items/enchants/r213_imprint_2h.dbr",        CanUseTransferAugment },
    { "grimleague/items/enchants/r214_imprint_shield.dbr",    CanUseTransferAugment },
    { "grimleague/items/enchants/r215_imprint_offhand.dbr",   CanUseTransferAugment },
};

std::vector<void*> GetSearchTabsList()
{
    std::vector<void*> searchTabs;
    void* mainPlayer = GameAPI::GetMainPlayer();
    const std::vector<void*>& personalTabs = GameAPI::GetPersonalTabs(mainPlayer);
    const std::vector<void*>& transferTabs = GameAPI::GetTransferTabs();

    searchTabs.insert(searchTabs.end(), personalTabs.begin(), personalTabs.end());
    searchTabs.insert(searchTabs.end(), transferTabs.begin(), transferTabs.end());

    void* inventory = GameAPI::GetPlayerInventory(mainPlayer);
    for (uint32_t i = 0; i < GameAPI::GetInventoryTabCount(inventory); ++i)
    {
        searchTabs.push_back(GameAPI::GetInventoryTab(inventory, i));
    }
    return searchTabs;
}

void ModifyEnchant(void* enchant, GameAPI::ItemReplicaInfo& enchantInfo)
{
    uint32_t enchantID = EngineAPI::GetObjectID(enchant);
    std::vector<void*> tabList = GetSearchTabsList();
    for (size_t i = 0; i < tabList.size(); ++i)
    {
        void* tab = tabList[i];
        const std::map<uint32_t, EngineAPI::Rect>& items = GameAPI::GetItemsInTab(tab);
        if (items.count(enchantID) > 0)
        {
            EngineAPI::Rect bounds = items.at(enchantID);
            EngineAPI::Vec2 position = EngineAPI::Vec2(bounds._x, bounds._y);

            if (GameAPI::RemoveItemFromTab(tab, enchantID))
            {
                EngineAPI::DestroyObjectEx(enchant);
                if (enchantInfo._itemStackCount > 1)
                {
                    enchantInfo._itemStackCount--;
                    void* newEnchant = GameAPI::CreateItem(enchantInfo);
                    GameAPI::AddItemToTab(tab, position, newEnchant, true);
                }
                break;
            }
        }
    }
}

bool ModifyItem(void* item, GameAPI::ItemReplicaInfo& itemInfo)
{
    uint32_t itemID = EngineAPI::GetObjectID(item);
    std::vector<void*> tabList = GetSearchTabsList();
    for (size_t i = 0; i < tabList.size(); ++i)
    {
        void* tab = tabList[i];
        const std::map<uint32_t, EngineAPI::Rect>& items = GameAPI::GetItemsInTab(tab);
        if (items.count(itemID) > 0)
        {
            EngineAPI::Rect bounds = items.at(itemID);
            EngineAPI::Vec2 position = EngineAPI::Vec2(bounds._x, bounds._y);

            if (GameAPI::RemoveItemFromTab(tab, itemID))
            {
                EngineAPI::DestroyObjectEx(item);
                void* newItem = GameAPI::CreateItem(itemInfo);
                return GameAPI::AddItemToTab(tab, position, newItem, true);
            }
        }
    }
    return false;
}

bool HandleUseItemEnchantment(void* _this, void* item, bool unk1, bool& unk2)
{
    typedef bool (__thiscall* UseItemEnchantmentProto)(void*, void*, bool, bool&);

    UseItemEnchantmentProto callback = (UseItemEnchantmentProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_USE_ITEM_ENCHANTMENT);
    if (callback)
    {
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        GameAPI::ItemReplicaInfo enchantInfo = GameAPI::GetItemReplicaInfo(_this);
        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(item);

        std::string enchantName = enchantInfo._itemName;
        if (itemCraftingHandlers.count(enchantName) > 0)
        {
            ItemCraftingHandler handler = itemCraftingHandlers.at(enchantName);
            if (handler(item, _this, itemInfo))
            {
                if (ModifyItem(item, itemInfo))
                {
                    ModifyEnchant(_this, enchantInfo);
                    GameAPI::SaveGame();
                }
            }
            return false;
        }
        return callback(_this, item, unk1, unk2);
    }
    return false;
}

bool HandleCanEnchantBeUsedOn(void* _this, void* item, bool unk1, bool& unk2)
{
    typedef bool (__thiscall* CanEnchantBeUsedOnProto)(void*, void*, bool, bool&);

    CanEnchantBeUsedOnProto callback = (CanEnchantBeUsedOnProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_CAN_ENCHANT_BE_USED_ON);
    if (callback)
    {
        bool result = callback(_this, item, unk1, unk2);

        if (_this != _lastEnchant)
        {
            _lastEnchant = _this;
            _lastEnchantInfo = GameAPI::GetItemReplicaInfo(_this);
        }

        auto pair = canUseItemFilters.find(_lastEnchantInfo._itemName);
        if (pair != canUseItemFilters.end())
        {
            CanUseItemFilter filter = pair->second;

            ItemType itemType = GameAPI::GetItemType(item);
            if (((1 << itemType) & 0xEBFE) == 0)    // This bitmask checks to make sure the item is equippable
                return false;

            // Prevent crafting on items that are already equipped
            // Use the cached equipment data if this is the same enchant as before
            if ((!_lastEquipment) || (_this != _lastEnchantUsedOn))
            {
                void* mainPlayer = GameAPI::GetMainPlayer();
                _lastEquipment = GameAPI::GetPlayerEquipment(mainPlayer);
                _lastEnchantUsedOn = _this;
            }

            uint32_t itemID = EngineAPI::GetObjectID(item);
            if (GameAPI::IsItemEquipped(_lastEquipment, itemID))
                return false;

            uint32_t itemLevel = GameAPI::GetItemLevel(item);
            GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(item);
            GameAPI::ItemClassification itemRarity = GameAPI::GetItemClassification(item);

            return filter(item, _this, itemLevel, itemInfo, itemType, itemRarity);
        }

        return result;
    }
    return false;
}
