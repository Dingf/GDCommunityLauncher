#include <sstream>
#include <iomanip>
#include "ItemDatabase.h"

ItemDatabase& ItemDatabase::GetInstance()
{
    static ItemDatabase instance;
    return instance;
}

void ItemDatabase::Load(const char* buffer, size_t size)
{
    if (_itemData.empty())
    {
        std::string str(buffer, size);
        std::stringstream ss(str);

        std::string name;
        uint32_t type;
        uint32_t width;
        uint32_t height;
        while (ss >> std::quoted(name, '"', '\0') >> type >> width >> height)
        {
            _itemData[name] = { (ItemType)type, width, height };
        }
    }
}