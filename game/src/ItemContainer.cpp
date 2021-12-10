#include "ItemContainer.h"
#include "ItemDatabase.h"
#include "Log.h"

ItemContainer::ItemContainer(uint32_t width, uint32_t height, bool ignoreItemSize) : _width(width), _height(height), _ignoreItemSize(ignoreItemSize)
{
    for (uint32_t i = 0; i < height; ++i)
    {
        _grid.emplace_back();
        for (uint32_t j = 0; j < width; ++j)
        {
            _grid[i].emplace_back(width, height);
        }
    }
}

ItemContainer::~ItemContainer()
{
    _itemList.clear();
}

void ItemContainer::AddItem(const Item& item, int32_t x, int32_t y)
{
    //TODO: 

    /*
    if ((x >= _width) || (y >= _height) || (x < 0) || (y < 0))
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Tried to add item \"%\" at coordinates (%, %), which is outside the bounds of the container. Item has not been added!", item->_itemName, x, y);
        return;
    }

    GridData& gridData = _grid[y][x];
    if (_ignoreItemSize)
    {
        if (gridData._item == nullptr)
        {
            gridData._item = item;
            gridData._real = true;
            _itemList[item.get()] = ((uint64_t)x << 32) | y;
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Tried to add item \"%\" at coordinates (%, %), which overlaps with an existing item. Item has not been added!", item->_itemName, x, y);
        }
    }
    else
    {
        uint32_t width;
        uint32_t height;
        try
        {
            ItemDatabase& database = ItemDatabase::GetInstance();
            const ItemDatabase::ItemDBEntry& entry = database.GetEntry(item->_itemName);
            width = entry._width;
            height = entry._height;
        }
        catch (std::out_of_range&)
        {
            // TODO: Re-enable this error message once item DB is fully supported
            Logger::LogMessage(LOG_LEVEL_WARN, "Tried to add item \"%\" that does not exist in item database. Item has not been added!", item->_itemName);
            return;
        }

        if ((gridData._item == nullptr) && (gridData._down - y + 1 >= height) && (gridData._right - x + 1 >= width))
        {
            for (uint32_t i = 0; i < height; ++i)
            {
                for (uint32_t j = 0; j < width; ++j)
                {
                    _grid[y + i][x + j]._item = item;
                    _grid[y + i][x + j]._down = 0;
                    _grid[y + i][x + j]._right = 0;
                    _grid[y + i][x + j]._real = (i == 0) && (j == 0);
                }
            }
            _itemList[item.get()] = ((uint64_t)x << 32) | y;

            if (x > 0)
            {
                for (uint32_t i = 0; i < height; ++i)
                {
                    int32_t j = x - 1;
                    while ((j >= 0) && (_grid[y+i][j]._item == nullptr))
                    {
                        _grid[y+i][j--]._right = x - 1;
                    }
                }
            }

            if (y > 0)
            {
                for (uint32_t j = 0; j < width; ++j)
                {
                    int32_t i = y - 1;
                    while ((i >= 0) && (_grid[i][x+j]._item == nullptr))
                    {
                        _grid[i--][x+j]._down = y - 1;
                    }
                }
            }
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Tried to add item \"%\" at coordinates (%, %), which overlaps with an existing item. Item has not been added!", item->_itemName, x, y);
        }
    }*/
}

void AddItemList(const std::vector<std::shared_ptr<Item>>& items)
{
    //TODO: Implement me
}