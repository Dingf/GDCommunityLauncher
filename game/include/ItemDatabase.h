#ifndef INC_GDCL_GAME_ITEM_DATABASE_H
#define INC_GDCL_GAME_ITEM_DATABASE_H

#include <map>
#include <string>

class ItemDatabase
{
    public:
        struct ItemDBEntry
        {
            uint32_t _width;
            uint32_t _height;
        };

        static ItemDatabase& ItemDatabase::GetInstance();

        bool IsLoaded() const { return !_database.empty(); }

        void Load(const char* buffer, size_t size);

        const ItemDBEntry& GetEntry(std::string name) { return _database.at(name); }

    private:
        std::map<std::string, ItemDBEntry> _database;
};


#endif//INC_GDCL_GAME_ITEM_DATABASE_H