#ifndef INC_GDCL_EXTRACTOR_CRAFTING_TABLE_DBR_H
#define INC_GDCL_EXTRACTOR_CRAFTING_TABLE_DBR_H

#include <map>
#include <filesystem>
#include "DBRecord.h"

class CraftingDBR : public DBRecord
{
    public:
        struct CraftingDBEntry
        {
            std::string _name;
            uint32_t    _minLevel;
            uint32_t    _maxLevel;
            uint32_t    _weight;
            bool        _rare;
        };

        uint32_t GetTotalValueWeight() const;

        CraftingDBR(const std::filesystem::path& path);

        static void BuildCraftingDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath);

    private:
        typedef std::map<int32_t, CraftingDBEntry> CraftingTable;

        struct ItemBaseLevel
        {
            ItemBaseLevel(uint32_t itemLevel, const std::string& itemRecordName) : _itemLevel(itemLevel), _itemRecordName(itemRecordName) {}

            bool operator<(const ItemBaseLevel& base) const { return (_itemLevel < base._itemLevel); }

            uint32_t    _itemLevel;
            std::string _itemRecordName;
        };

        void LoadValue(std::string key, std::unique_ptr<Value> value);

        CraftingTable _prefixes;
        CraftingTable _suffixes;
        CraftingTable _values;
};

#endif//INC_GDCL_EXTRACTOR_CRAFTING_TABLE_DBR_H