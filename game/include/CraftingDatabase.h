#ifndef INC_GDCL_GAME_CRAFTING_DATABASE_H
#define INC_GDCL_GAME_CRAFTING_DATABASE_H

#include <string>
#include <unordered_map>
#include "ItemType.h"

class CraftingDatabase
{
    public:
        struct CraftingDBEntry
        {
            CraftingDBEntry(std::string name, std::string value, uint32_t minLevel, uint32_t maxLevel, uint32_t weight)
                : _name(name), _value(value), _minLevel(minLevel), _maxLevel(maxLevel), _weight(weight) {}

            CraftingDBEntry() : _name({}), _value({}), _minLevel(0), _maxLevel(0), _weight(0) {}

            std::string _name;
            std::string _value;
            uint32_t    _minLevel;
            uint32_t    _maxLevel;
            uint32_t    _weight;
        };

        typedef std::unordered_map<std::string, CraftingDBEntry> CraftingDBTable;

        static CraftingDatabase& GetInstance();

        bool IsLoaded() const { return !_craftingData.empty(); }
        bool HasTable(const std::string& name) const { return (_craftingData.count(name) > 0); }

        uint32_t GetTableWeight(const std::string& name) const { return _craftingWeight.at(name); }
        const CraftingDBTable& GetTable(const std::string& name) const { return _craftingData.at(name); }

        void Load(const char* buffer, size_t size);

    private:
        CraftingDatabase() {}
        CraftingDatabase(CraftingDatabase&) = delete;
        void operator=(const CraftingDatabase&) = delete;

        std::unordered_map<std::string, uint32_t> _craftingWeight;
        std::unordered_map<std::string, CraftingDBTable> _craftingData;

};

#endif//INC_GDCL_GAME_CRAFTING_DATABASE_H