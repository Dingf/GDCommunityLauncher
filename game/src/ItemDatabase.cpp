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
    if (_database.empty())
    {
        std::string buffer(buffer, size);
        std::stringstream ss(buffer);

        std::string name;
        uint32_t width;
        uint32_t height;
        while (ss >> std::quoted(name, '"', '\0') >> width >> height)
        {
            _database[name] = { width, height };
        }
    }
}