#ifndef INC_GDCL_GAME_ITEM_CONTAINER_H
#define INC_GDCL_GAME_ITEM_CONTAINER_H

#include <memory>
#include <map>
#include <vector>
#include <list>
#include "Item.h"

enum ItemContainerType
{
    ITEM_CONTAINER_INVALID,
    ITEM_CONTAINER_SHARED_STASH,
    ITEM_CONTAINER_CRAFTING_STASH,  // Unused
    ITEM_CONTAINER_CHAR_STASH,
    ITEM_CONTAINER_CHAR_INVENTORY,
    ITEM_CONTAINER_CHAR_BAG,
    ITEM_CONTAINER_CHAR_EQUIP
};

class ItemContainer
{
    public:
        typedef std::map<std::shared_ptr<Item>, uint64_t> ItemList;

        ItemContainer(uint32_t width, uint32_t height);
        virtual ~ItemContainer() = 0;

        uint32_t GetWidth()  const { return _width; }
        uint32_t GetHeight() const { return _height; }

        size_t GetItemCount() const { return _itemList.size(); }

        virtual ItemContainerType GetContainerType() const = 0;

        bool AddItem(const Item& item);
        bool AddItem(const Item& item, uint32_t x, uint32_t y);
        std::vector<Item*> AddItemList(const std::vector<Item>& items);

              ItemList& GetItemList()       { return _itemList; }
        const ItemList& GetItemList() const { return _itemList; }

    protected:
        const uint32_t _width;
        const uint32_t _height;

        struct GridData
        {
            GridData() : _item(nullptr), _right(0), _down(0), _real(false) {}
            GridData(int32_t right, int32_t down) : _item(nullptr), _right(right), _down(down), _real(false) {}

            Item* _item;        // The pointer to the item, or nullptr if empty
            int32_t _right;     // The farthest continuous empty index to the right
            int32_t _down;      // The farthest continuous empty index to the bottom
            bool _real;         // Whether or not the grid data contains the actual location of the item
        };

        std::vector<GridData> _grid;    // The array of width * height GridData structures, used to store the state of the container
        std::list<uint32_t> _next;      // A sorted list of next indices to try when placing an item
        ItemList _itemList;             // A mapping of each item to a fixed index (not the same as the width/height index used by _grid and _next)
};

#endif//INC_GDCL_GAME_ITEM_CONTAINER_H