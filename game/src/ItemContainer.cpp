#include "ItemContainer.h"

ItemContainer::ItemContainer(uint32_t width, uint32_t height, bool ignoreItemSize) : _width(width), _height(height), _ignoreItemSize(ignoreItemSize)
{
    for (uint32_t i = 0; i < width * height; ++i)
    {
        _grid.emplace_back(width, height);
        _next.emplace_back(i);
    }
}

ItemContainer::~ItemContainer()
{
    _itemList.clear();
    _next.clear();
}

bool ItemContainer::AddItem(const Item& item)
{
    for (auto it = _next.begin(); it != _next.end();)
    {
        uint32_t index = *it;
        uint32_t x = index % _width;
        uint32_t y = index / _width;

        if (_grid[index]._item != nullptr)
        {
            it = _next.erase(it);
        }
        else if (AddItem(item, x, y))
        {
            _next.erase(it);
            return true;
        }
        else
        {
            it++;
        }
    }

    return false;
}

bool ItemContainer::AddItem(const Item& item, uint32_t x, uint32_t y)
{
    if ((x >= _width) || (y >= _height))
        return false;

    if (_ignoreItemSize)
    {
        uint32_t index = (y  * _width) + x;
        if (_grid[index]._item == nullptr)
        {
            std::shared_ptr<Item> itemCopy = std::make_shared<Item>(item);
            _itemList[itemCopy] = ((uint64_t)x << 32) | y;
            _grid[index]._item = itemCopy.get();
            _grid[index]._real = true;
            return true;
        }
    }
    else if (CanPlaceItem(item, x, y))
    {
        uint32_t itemWidth = item._itemWidth;
        uint32_t itemHeight = item._itemHeight;

        std::shared_ptr<Item> itemCopy = std::make_shared<Item>(item);
        _itemList[itemCopy] = ((uint64_t)x << 32) | y;

        for (uint32_t i = 0; i < itemHeight; i++)
        {
            for (uint32_t j = 0; j < itemWidth; j++)
            {
                uint32_t index = ((y + i) * _width) + (x + j);
                _grid[index]._item  = itemCopy.get();
                _grid[index]._down  = -1;
                _grid[index]._right = -1;
                _grid[index]._real  = ((i == 0) && (j == 0));
            }
        }

        for (uint32_t i = 0; i < itemHeight; ++i)
        {
            for (int32_t j = x - 1; j >= 0; j--)
            {
                uint32_t index = ((y + i) * _width) + j;
                if (_grid[index]._item == nullptr)
                    _grid[index]._right = x;
                else
                    break;
            }
        }

        for (uint32_t i = 0; i < itemWidth; ++i)
        {
            for (int32_t j = y - 1; j >= 0; j--)
            {
                uint32_t index = (j * _width) + (x + i);
                if (_grid[index]._item == nullptr)
                    _grid[index]._down = y;
                else
                    break;
            }
        }
        return true;
    }
    return false;
}

std::vector<Item*> ItemContainer::AddItemList(const std::vector<Item>& items)
{
    std::vector<Item*> result;
    for (int32_t i = items.size() - 1; i >= 0; i--)
    {
        const Item& item = items[i];
        for (auto it = _next.begin(); it != _next.end();)
        {
            uint32_t index = *it;
            uint32_t x = index % _width;
            uint32_t y = index / _width;

            if (_grid[index]._item != nullptr)
            {
                it = _next.erase(it);
            }
            else if (AddItem(item, x, y))
            {
                result.emplace_back(_grid[index]._item);
                _next.erase(it);
                break;
            }
            else
            {
                it++;
            }
        }
    }
    return result;
}

bool ItemContainer::CanPlaceItem(const Item& item, uint32_t x, uint32_t y) const
{
    uint32_t itemWidth = item._itemWidth;
    uint32_t itemHeight = item._itemHeight;

    if ((x + itemWidth > _width) || (y + itemHeight > _height))
        return false;

    for (uint32_t i = 0; i < itemWidth && i < itemHeight; i++)
    {
        uint32_t index = ((y + i) * _width) + (x + i);
        if ((_grid[index]._item != nullptr) || (_grid[index]._right <= 0) || (_grid[index]._down <= 0) || (_grid[index]._right < (x + itemWidth)) || (_grid[index]._down < (y + itemHeight)))
            return false;
    }
    return true;
}