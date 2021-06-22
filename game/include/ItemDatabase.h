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

        static const std::map<std::string, ItemDBEntry>& GetDatabase() { return _database; }

    private:
        static const std::map<std::string, ItemDBEntry> _database;
};


#endif//INC_GDCL_GAME_ITEM_DATABASE_H