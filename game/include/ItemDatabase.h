#ifndef INC_GDCL_GAME_ITEM_DATABASE_H
#define INC_GDCL_GAME_ITEM_DATABASE_H

#include <unordered_map>
#include <string>
#include "ItemType.h"

class ItemDatabase
{
    public:
        struct ItemDBEntry
        {
            ItemType _type;
            uint32_t _width;
            uint32_t _height;
        };

        static ItemDatabase& GetInstance();

        bool IsLoaded() const { return !_itemData.empty(); }
        bool HasEntry(const std::string& name) const { return (_itemData.count(name) > 0); }

        const ItemDBEntry& GetEntry(const std::string& name) const { return _itemData.at(name); }

        void Load(const char* buffer, size_t size);

    private:
        ItemDatabase() {}
        ItemDatabase(ItemDatabase&) = delete;
        void operator=(const ItemDatabase&) = delete;

        std::unordered_map<std::string, ItemDBEntry> _itemData;
};


#endif//INC_GDCL_GAME_ITEM_DATABASE_H