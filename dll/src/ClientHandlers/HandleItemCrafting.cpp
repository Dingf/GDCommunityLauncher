#include <unordered_map>
#include <random>
#include <regex>
#include "ClientHandlers.h"
#include "CraftingDatabase.h"
#include "ItemDatabase.h"
#include "ServerSync.h"

std::unordered_map<std::string, uint32_t> _imprintTypeMap =
{
    { "grimleague/items/enchants/r201_imprint_helm.dbr",      ITEM_TYPE_FLAG_HEAD },
    { "grimleague/items/enchants/r202_imprint_torso.dbr",     ITEM_TYPE_FLAG_CHEST },
    { "grimleague/items/enchants/r203_imprint_gloves.dbr",    ITEM_TYPE_FLAG_HANDS },
    { "grimleague/items/enchants/r204_imprint_shoulders.dbr", ITEM_TYPE_FLAG_SHOULDERS },
    { "grimleague/items/enchants/r205_imprint_pants.dbr",     ITEM_TYPE_FLAG_LEGS },
    { "grimleague/items/enchants/r206_imprint_boots.dbr",     ITEM_TYPE_FLAG_FEET },
    { "grimleague/items/enchants/r207_imprint_belt.dbr",      ITEM_TYPE_FLAG_BELT },
    { "grimleague/items/enchants/r208_imprint_medal.dbr",     ITEM_TYPE_FLAG_MEDAL },
    { "grimleague/items/enchants/r209_imprint_ring.dbr",      ITEM_TYPE_FLAG_RING },
    { "grimleague/items/enchants/r210_imprint_amulet.dbr",    ITEM_TYPE_FLAG_AMULET },
    { "grimleague/items/enchants/r211_imprint_1h.dbr",        ITEM_TYPE_FLAG_SWORD_1H | ITEM_TYPE_FLAG_AXE_1H | ITEM_TYPE_FLAG_MACE_1H | ITEM_TYPE_FLAG_RANGED_1H },
    { "grimleague/items/enchants/r212_imprint_1hcaster.dbr",  ITEM_TYPE_FLAG_DAGGER | ITEM_TYPE_FLAG_FLAG_SCEPTER },
    { "grimleague/items/enchants/r213_imprint_2h.dbr",        ITEM_TYPE_FLAG_SWORD_2H | ITEM_TYPE_FLAG_AXE_2H | ITEM_TYPE_FLAG_MACE_2H | ITEM_TYPE_FLAG_RANGED_2H },
    { "grimleague/items/enchants/r214_imprint_shield.dbr",    ITEM_TYPE_FLAG_SHIELD },
    { "grimleague/items/enchants/r215_imprint_offhand.dbr",   ITEM_TYPE_FLAG_OFFHAND },
};

std::random_device dev;
std::mt19937 random(dev());

const std::regex vaalRegex("^grimleague/items/lootaffixes/ultos_prefix(\\d{2}).dbr$");

void* _lastEnchantUsedOn = nullptr;
void* _lastEquipment = nullptr;
void* _lastEnchant = nullptr;
GameAPI::ItemReplicaInfo _lastEnchantInfo;

uint32_t GenerateItemSeed()
{
    std::uniform_int_distribution<std::mt19937::result_type> dist(0x00000000, 0xFFFFFFFF);
    return dist(random);
}

std::string GenerateAffixRoll(const std::string& tableName, ItemType itemType, uint32_t itemLevel, std::string ignore)
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

        std::uniform_int_distribution<std::mt19937::result_type> dist(0, totalWeight);
        int64_t rollWeight = dist(random);
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

std::string GenerateItemPrefix(ItemType itemType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "prefix" + std::to_string(itemType);
    return GenerateAffixRoll(tableName, itemType, itemLevel, ignore);
}

std::string GenerateItemSuffix(ItemType itemType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "suffix" + std::to_string(itemType);
    return GenerateAffixRoll(tableName, itemType, itemLevel, ignore);
}

std::string GenerateVaalAffix(ItemType itemType, uint32_t itemLevel, std::string ignore)
{
    std::string tableName = "vaal" + std::to_string(itemType);
    return GenerateAffixRoll(tableName, itemType, itemLevel, ignore);
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

bool HasVaalAffix(void* item)
{
    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    return (std::regex_match(itemInfo._itemPrefix, vaalRegex) || std::regex_match(itemInfo._itemSuffix, vaalRegex));
}

bool CanRerollItemSeed(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalAffix(item));
}

bool CanRerollItemAffix(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalAffix(item) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE)) && (itemType < ITEM_TYPE_RELIC);
}

bool CanRerollItemAffixLowLevel(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalAffix(item) && (itemRarity <= GameAPI::ITEM_CLASSIFICATION_RARE) && (itemLevel <= 70)) && (itemType < ITEM_TYPE_RELIC);
}

bool CanRerollEpicAffix(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return (!HasVaalAffix(item) && (itemRarity == GameAPI::ITEM_CLASSIFICATION_EPIC)) && (itemType < ITEM_TYPE_RELIC);
}

bool CanRerollVaal(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    return ((itemRarity == GameAPI::ITEM_CLASSIFICATION_EPIC) || (itemRarity == GameAPI::ITEM_CLASSIFICATION_LEGEND)) && (itemType < ITEM_TYPE_RELIC);
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

bool CanUpgradeItemBase(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalAffix(item) || (itemType == ITEM_TYPE_RELIC))
        return false;

    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "upgrade";
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).count(itemInfo._itemName) > 0);
    }
    return false;
}

bool CanDowngradeItemBase(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalAffix(item) || (itemType == ITEM_TYPE_RELIC))
        return false;

    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();

    std::string tableName = "downgrade";
    if (craftingDB.HasTable(tableName))
    {
        return (craftingDB.GetTable(tableName).count(itemInfo._itemName) > 0);
    }
    return false;
}

bool CanUpgradeItemBase(void* item)
{
    return (!HasVaalAffix(item) && CanUpgradeItemBase(item, nullptr, 0, ITEM_TYPE_OTHER, GameAPI::ITEM_CLASSIFICATION_NORMAL));
}

bool CanDowngradeItemBase(void* item)
{
    return (!HasVaalAffix(item) && CanDowngradeItemBase(item, nullptr, 0, ITEM_TYPE_OTHER, GameAPI::ITEM_CLASSIFICATION_NORMAL));
}

bool CanUpgradeItemBaseLowLevel(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    return (CanUpgradeItemBase(item) && (itemLevel <= 49));
}

bool CanCreateTransferAugment(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    return (!HasVaalAffix(item) && (itemRarity < GameAPI::ITEM_CLASSIFICATION_LEGEND) && (itemType < ITEM_TYPE_RELIC) && (!itemInfo._itemPrefix.empty() || !itemInfo._itemSuffix.empty()));
}

bool CanUseTransferAugment(void* item, void* enchant, uint32_t itemLevel, ItemType itemType, GameAPI::ItemClassification itemRarity)
{
    if (HasVaalAffix(item))
        return false;

    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    GameAPI::ItemReplicaInfo itemInfo;
    GameAPI::ItemReplicaInfo enchantInfo;
    GameAPI::GetItemReplicaInfo(item, itemInfo);
    GameAPI::GetItemReplicaInfo(enchant, enchantInfo);

    // Prevent transfers on legendary items and on epic items if the augment has both a prefix and a suffix
    if ((itemType == ITEM_TYPE_RELIC) || (itemRarity >= GameAPI::ITEM_CLASSIFICATION_LEGEND) || ((itemRarity == GameAPI::ITEM_CLASSIFICATION_EPIC) && (!enchantInfo._itemPrefix.empty()) && (!enchantInfo._itemSuffix.empty())))
        return false;

    std::string enchantName = enchantInfo._itemName;
    if (_imprintTypeMap.count(enchantName) > 0)
    {
        uint32_t typeFlag = _imprintTypeMap.at(enchantName);
        return ((typeFlag & (1 << (itemType - 1))) != 0);
    }
    return false;
}

bool HandleRerollItemSeed(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    itemInfo._itemSeed = GenerateItemSeed();
    return true;
}

bool HandleRerollItemPrefix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if (itemDB.HasEntry(itemName))
    {
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        ItemDatabase::ItemDBEntry entry = itemDB.GetEntry(itemInfo._itemName);
        itemInfo._itemPrefix = GenerateItemPrefix(entry._type, itemLevel, itemInfo._itemPrefix);
        return true;
    }
    return false;
}

bool HandleRerollItemSuffix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if (itemDB.HasEntry(itemName))
    {
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        ItemDatabase::ItemDBEntry entry = itemDB.GetEntry(itemInfo._itemName);
        itemInfo._itemSuffix = GenerateItemSuffix(entry._type, itemLevel, itemInfo._itemSuffix);
        return true;
    }
    return false;
}

bool HandleRerollEpicAffix(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if (itemDB.HasEntry(itemName))
    {
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        ItemDatabase::ItemDBEntry entry = itemDB.GetEntry(itemName);
        uint32_t roll = GenerateItemSeed();
        if ((roll % 2) == 0)
        {
            itemInfo._itemPrefix = GenerateItemPrefix(entry._type, itemLevel, itemInfo._itemSuffix);
            itemInfo._itemSuffix = "";
        }
        else
        {
            itemInfo._itemPrefix = "";
            itemInfo._itemSuffix = GenerateItemSuffix(entry._type, itemLevel, itemInfo._itemSuffix);
        }
        return true;
    }
    return false;
}

bool HandleUpgradeItemBase(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    if (CanUpgradeItemBase(item))
    {
        itemInfo._itemName = UpgradeItemBase(itemName);
        return true;
    }
    return false;
}

bool HandleDowngradeItemBase(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    if (CanDowngradeItemBase(item))
    {
        itemInfo._itemName = DowngradeItemBase(itemName);
        return true;
    }
    return false;
}

bool HandleRerollVaal(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{

    std::string itemName = itemInfo._itemName;
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if (itemDB.HasEntry(itemName))
    {
        ItemDatabase::ItemDBEntry entry = itemDB.GetEntry(itemName);
        uint32_t roll = GenerateItemSeed() % 100;
        if (roll < 25)
        {
            // Brick the item, e.g. turn it into Scrap/Aether Crystal
            itemInfo._itemPrefix = "";
            itemInfo._itemSuffix = "";
            itemInfo._itemModifier = "";
            if ((entry._width >= 2) && (entry._height >= 2))
            {
                itemInfo._itemName = "records/items/questitems/scrapmetal.dbr";
            }
            else
            {
                itemInfo._itemName = "records/items/materia/compa_aethercrystal.dbr";
            }
        }
        else
        {
            // Add a special crafted bonus affix
            uint32_t itemLevel = GameAPI::GetItemLevel(item);

            // Prioritize overwriting existing bonus affixes over empty prefix/suffix slots
            if (std::regex_match(itemInfo._itemPrefix, vaalRegex))
                itemInfo._itemPrefix = GenerateVaalAffix(entry._type, itemLevel, itemInfo._itemPrefix);
            else if (std::regex_match(itemInfo._itemSuffix, vaalRegex))
                itemInfo._itemSuffix = GenerateVaalAffix(entry._type, itemLevel, itemInfo._itemSuffix);
            else if (itemInfo._itemPrefix.empty())
                itemInfo._itemPrefix = GenerateVaalAffix(entry._type, itemLevel, {});
            else
                itemInfo._itemSuffix = GenerateVaalAffix(entry._type, itemLevel, {});
        }
        return true;
    }
    return false;
}

bool HandleCreateTransferAugment(void* item, void* enchant, GameAPI::ItemReplicaInfo& itemInfo)
{
    std::string itemName = itemInfo._itemName;
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if ((!itemInfo._itemPrefix.empty() || !itemInfo._itemSuffix.empty()) && (itemDB.HasEntry(itemName)))
    {
        ItemDatabase::ItemDBEntry entry = itemDB.GetEntry(itemName);
        ItemType itemType = entry._type;
        if (itemType != ITEM_TYPE_OTHER)
        {
            uint32_t itemTypeFlag = 1 << (itemType - 1);
            for (const auto& pair : _imprintTypeMap)
            {
                if ((pair.second & itemTypeFlag) != 0)
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
    GameAPI::ItemReplicaInfo enchantInfo;
    GameAPI::GetItemReplicaInfo(enchant, enchantInfo);

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
    { "grimleague/items/enchants/r107_vaal_epic.dbr",         HandleRerollEpicAffix },
    { "grimleague/items/enchants/r108_vaal_legendary.dbr",    HandleRerollVaal },
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

typedef bool (*CanUseItemFilter)(void*, void*, uint32_t, ItemType, GameAPI::ItemClassification);
std::unordered_map<std::string, CanUseItemFilter> canUseItemFilters =
{
    { "grimleague/items/enchants/r101_pre.dbr",               CanRerollItemAffix },
    { "grimleague/items/enchants/r102_suff.dbr",              CanRerollItemAffix },
    { "grimleague/items/enchants/r103_pre_70.dbr",            CanRerollItemAffixLowLevel },
    { "grimleague/items/enchants/r104_suff_70.dbr",           CanRerollItemAffixLowLevel },
    { "grimleague/items/enchants/r105_seed.dbr",              CanRerollItemSeed },
    { "grimleague/items/enchants/r106_upgrade.dbr",           CanUpgradeItemBase },
    { "grimleague/items/enchants/r106a_upgrade.dbr",          CanUpgradeItemBaseLowLevel },
    { "grimleague/items/enchants/r107_vaal_epic.dbr",         CanRerollEpicAffix },
    { "grimleague/items/enchants/r108_vaal_legendary.dbr",    CanRerollVaal },
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

    UseItemEnchantmentProto callback = (UseItemEnchantmentProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_USE_ITEM_ENCHANTMENT);
    if (callback)
    {
        uint32_t itemLevel = GameAPI::GetItemLevel(item);
        GameAPI::ItemReplicaInfo enchantInfo;
        GameAPI::ItemReplicaInfo itemInfo;

        GameAPI::GetItemReplicaInfo(_this, enchantInfo);
        GameAPI::GetItemReplicaInfo(item, itemInfo);

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

    CanEnchantBeUsedOnProto callback = (CanEnchantBeUsedOnProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_CAN_ENCHANT_BE_USED_ON);
    if (callback)
    {
        bool result = callback(_this, item, unk1, unk2);

        if (_this != _lastEnchant)
        {
            _lastEnchant = _this;
            GameAPI::GetItemReplicaInfo(_this, _lastEnchantInfo);
        }

        std::string enchantName = _lastEnchantInfo._itemName;
        if (canUseItemFilters.count(enchantName) > 0)
        {
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

            CanUseItemFilter filter = canUseItemFilters.at(enchantName);

            ItemDatabase& itemDB = ItemDatabase::GetInstance();
            GameAPI::ItemReplicaInfo itemInfo;
            GameAPI::GetItemReplicaInfo(item, itemInfo);

            std::string itemName = itemInfo._itemName;
            if (itemDB.HasEntry(itemName))
            {
                uint32_t itemLevel = GameAPI::GetItemLevel(item);
                ItemType itemType = itemDB.GetEntry(itemInfo._itemName)._type;
                GameAPI::ItemClassification itemRarity = GameAPI::GetItemClassification(item);

                if (itemType != ITEM_TYPE_OTHER)
                {
                    return filter(item, _this, itemLevel, itemType, itemRarity);
                }
            }
            return false;
        }

        return result;
    }
    return false;
}
