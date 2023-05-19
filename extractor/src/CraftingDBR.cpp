#include <unordered_map>
#include <set>
#include <algorithm>
#include <regex>
#include <filesystem>
#include "CraftingTemplate.h"
#include "CraftingDBR.h"
#include "ItemDBR.h"
#include "ItemType.h"
#include "Log.h"

// TODO: Delete me
#include <iostream>

const std::regex craftingEntryRegex("^(randomizer|prefixTable|rarePrefixTable|suffixTable|rareSuffixTable)([A-Za-z]+)(\\d+)$");

std::vector<std::regex> craftingBlacklist =
{
    std::regex("enemygear"),
    std::regex("npcgear"),
    std::regex("nodrop"),
    std::regex("common"),
    std::regex("ghost_visuals"),
    std::regex("illidan"),
    std::regex("hands_lesaunt"),
    std::regex("\\/[a-z][0-9]00_[A-Za-z0-9_-]+\\.dbr$"),
    std::regex("\\/a[0-9]+_[A-Za-z0-9_-]+\\.dbr$"),
    std::regex("\\/b02_iceskeletonspirit_common[a-z]\\.dbr$"),
    std::regex("\\/b_luminari_bayonettrifle_[0-9]+\\.dbr$"),
    std::regex("\\/b_luminari_bayonettrifle_[0-9]+\\.dbr$"),
};

uint32_t CraftingDBR::GetTotalValueWeight() const
{
    uint32_t sum = 0;
    for (const auto& pair : _values)
    {
        sum += pair.second._weight;
    }
    return sum;
}

void CraftingDBR::BuildCraftingDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath)
{
    if (!std::filesystem::is_directory(dataPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", dataPath.string().c_str()));

    if (!std::filesystem::is_directory(outPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", outPath.string().c_str()));

    std::filesystem::path outFilePath = outPath / "CraftingDatabase.txt";

    std::ofstream out(outFilePath, std::ofstream::out);
    if (!out.is_open())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open file % for writing", outFilePath.string().c_str()));

    for (uint32_t i = 1; i < MAX_ITEM_TYPES; ++i)
    {
        const CraftingTemplate& craftTemplate = CraftingTemplate::GetTemplate((ItemType)i);
        CraftingDBR templateDBR(dataPath / craftTemplate._recordName);

        for (const auto& tablePair : templateDBR._prefixes)
        {
            CraftingDBEntry table = tablePair.second;
            CraftingDBR tableDBR(dataPath / table._name);

            uint32_t tableTotalWeight = tableDBR.GetTotalValueWeight();
            for (const auto& entryPair : tableDBR._values)
            {
                CraftingDBEntry entry = entryPair.second;
                uint32_t adjustedMinLevel = std::max(entry._minLevel, table._minLevel);
                uint32_t adjustedMaxLevel = std::min(entry._maxLevel, table._maxLevel);
                uint32_t adjustedWeight = (table._weight * entry._weight * 100) / tableTotalWeight;
                if (table._rare)
                    adjustedWeight *= craftTemplate._prefixAdjustment;

                if ((adjustedWeight > 0) && (adjustedMinLevel <= adjustedMaxLevel))
                {
                    out << "\"prefix" << i << "\" "
                        << "\"" << entry._name << "\" "
                        << "\"\" "
                        << adjustedMinLevel << " "
                        << adjustedMaxLevel << " "
                        << adjustedWeight << "\n";
                }
            }
        }

        for (const auto& tablePair : templateDBR._suffixes)
        {
            CraftingDBEntry table = tablePair.second;
            CraftingDBR tableDBR(dataPath / table._name);

            uint32_t tableTotalWeight = tableDBR.GetTotalValueWeight();
            for (const auto& entryPair : tableDBR._values)
            {
                CraftingDBEntry entry = entryPair.second;
                uint32_t adjustedMinLevel = std::max(entry._minLevel, table._minLevel);
                uint32_t adjustedMaxLevel = std::min(entry._maxLevel, table._maxLevel);
                uint32_t adjustedWeight = (table._weight * entry._weight * 100) / tableTotalWeight;
                if (table._rare)
                    adjustedWeight *= craftTemplate._suffixAdjustment;

                if ((adjustedWeight > 0) && (adjustedMinLevel <= adjustedMaxLevel))
                {
                    out << "\"suffix" << i << "\" "
                        << "\"" << entry._name << "\" "
                        << "\"\" "
                        << adjustedMinLevel << " "
                        << adjustedMaxLevel << " "
                        << adjustedWeight << "\n";
                }
            }
        }
    }

    std::unordered_map<std::string, std::vector<ItemBaseLevel>> itemBaseMap;
    const std::vector<std::string>& searchPaths = ItemDBR::GetSearchPaths();
    for (uint32_t i = 0; i < searchPaths.size(); ++i)
    {
        std::filesystem::path searchPath = dataPath / searchPaths[i];
        for (auto& entry : std::filesystem::recursive_directory_iterator(searchPath))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".dbr"))
            {
                ItemDBR itemDBR(entry.path(), false);
                const Value* itemLevelValue = itemDBR.GetVariable("itemLevel");
                const Value* itemNameTagValue = itemDBR.GetVariable("itemNameTag");
                if ((itemLevelValue) && (itemNameTagValue))
                {
                    std::string itemNameTag = itemNameTagValue->ToString();
                    std::string itemRecordName = itemDBR.GetRecordPath().string();
                    std::replace(itemRecordName.begin(), itemRecordName.end(), '\\', '/');

                    bool blacklisted = false;
                    for (std::regex& blacklist : craftingBlacklist)
                    {
                        if (std::regex_search(itemRecordName, blacklist))
                        {
                            blacklisted = true;
                            break;
                        }
                    }

                    if (!blacklisted)
                        itemBaseMap[itemNameTag].emplace_back((uint32_t)itemLevelValue->ToInt(), itemRecordName);
                }
            }
        }
    }

    for (auto& pair : itemBaseMap)
    {
        if (pair.second.size() > 1)
        {
            std::sort(pair.second.begin(), pair.second.end());
            for (size_t i = 0; (i + 1) < pair.second.size(); ++i)
            {
                uint32_t itemLevel = pair.second[i]._itemLevel;
                uint32_t upgradeLevel = pair.second[i+1]._itemLevel;
                std::string itemRecordName = pair.second[i]._itemRecordName;
                std::string upgradeRecordName = pair.second[i+1]._itemRecordName;

                if (upgradeLevel > itemLevel)
                {
                    out << "\"upgrade\" "
                        << "\"" << itemRecordName << "\" "
                        << "\"" << upgradeRecordName << "\" "
                        << itemLevel << " "
                        << upgradeLevel << " "
                        << 1 << "\n";
                }
            }
        }
    }

    out.close();
}

CraftingDBR::CraftingDBR(const std::filesystem::path& path)
{
    if (!DBRecord::Load(path))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid crafting table DBR file", path.string().c_str()));
}

void CraftingDBR::LoadValue(std::string key, std::unique_ptr<Value> value)
{
    std::smatch match;
    if (std::regex_match(key, match, craftingEntryRegex))
    {
        std::string keyName = match.str(1);
        std::string entryType = match.str(2);
        int32_t entryID = (uint32_t)std::stoi(match.str(3));
        float multiplier = 1.0f;

        CraftingTable* table = nullptr;
        if (keyName == "randomizer")
            table = &_values;
        else if ((keyName == "prefixTable") || (keyName == "rarePrefixTable"))
            table = &_prefixes;
        else if ((keyName == "suffixTable") || (keyName == "rareSuffixTable"))
            table = &_suffixes;

        // Prevent rare/magic affixes from using the same ID and also apply a weight penalty for rare affixes
        if ((keyName == "rarePrefixTable") || (keyName == "rareSuffixTable"))
        {
            entryID = -entryID;
            table->operator[](entryID)._rare = true;
        }

        if (table != nullptr)
        {
            if (entryType == "Name")
                table->operator[](entryID)._name = value->ToString();
            else if (entryType == "LevelMin")
                table->operator[](entryID)._minLevel = (uint32_t)(std::max(value->ToInt(), 0));
            else if (entryType == "LevelMax")
                table->operator[](entryID)._maxLevel = (uint32_t)(std::max(value->ToInt(), 0));
            else if (entryType == "Weight")
                table->operator[](entryID)._weight = (uint32_t)(std::max(value->ToInt(), 0) * multiplier);
        }
    }
}