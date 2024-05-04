#include "ItemContainer.h"

#include "Log.h"

ItemContainer::ItemContainer(uint32_t width, uint32_t height) : _width(width), _height(height)
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

    uint32_t index = (y  * _width) + x;
    if (_grid[index]._item == nullptr)
    {
        std::shared_ptr<Item> itemCopy = std::make_shared<Item>(item);
        _itemList[itemCopy] = ((uint64_t)x << 32) | y;
        _grid[index]._item = itemCopy.get();
        _grid[index]._real = true;
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