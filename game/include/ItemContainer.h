#ifndef INC_GDCL_GAME_ITEM_CONTAINER_H
#define INC_GDCL_GAME_ITEM_CONTAINER_H

#include <memory>
#include <map>
#include <vector>
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
        ItemContainer(uint32_t width, uint32_t height, bool ignoreItemSize = false);
        virtual ~ItemContainer() = 0;

        uint32_t GetWidth()  const { return _width; }
        uint32_t GetHeight() const { return _height; }

        size_t GetItemCount() const { return _itemList.size(); }

        virtual ItemContainerType GetContainerType() const = 0;

        void AddItem(const Item& item);
        void AddItem(const Item& item, int32_t x, int32_t y);
        void AddItemList(const std::vector<Item>& items);

              std::map<Item*, uint64_t>& GetItemList()       { return _itemList; }
        const std::map<Item*, uint64_t>& GetItemList() const { return _itemList; }

    protected:
        const bool _ignoreItemSize;         // If specified, ignore item size when adding items

        const uint32_t _width;
        const uint32_t _height;

        struct GridData
        {
            GridData() : _item(nullptr), _right(0), _down(0), _real(false) {}
            GridData(int32_t right, int32_t down) : _item(nullptr), _right(right), _down(down), _real(false) {}

            std::shared_ptr<Item> _item;    // The pointer to the item, or nullptr if empty
            int32_t _right;                 // The farthest continuous empty index to the right
            int32_t _down;                  // The farthest continuous empty index to the bottom
            bool _real;                     // Whether or not the grid data contains the actual location of the item
        };

        std::vector<std::vector<GridData>> _grid;
        std::map<Item*, uint64_t> _itemList;

};

#endif//INC_GDCL_GAME_ITEM_CONTAINER_H