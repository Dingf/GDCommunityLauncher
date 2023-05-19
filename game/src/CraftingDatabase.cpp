#include <sstream>
#include <iomanip>
#include "CraftingDatabase.h"

CraftingDatabase& CraftingDatabase::GetInstance()
{
    static CraftingDatabase instance;
    return instance;
}

void CraftingDatabase::Load(const char* buffer, size_t size)
{
    if (_craftingData.empty())
    {
        std::string str(buffer, size);
        std::stringstream ss(str);

        std::string table;
        std::string name;
        std::string value;
        uint32_t minLevel;
        uint32_t maxLevel;
        uint32_t weight;
        while (ss >> std::quoted(table, '"', '\0') >> std::quoted(name, '"', '\0') >> std::quoted(value, '"', '\0') >> minLevel >> maxLevel >> weight)
        {
            _craftingData[table].try_emplace(name, name, value, minLevel, maxLevel, weight);
            if (_craftingWeight.count(table) == 0)
                _craftingWeight[table] = weight;
            else
                _craftingWeight[table] += weight;
        }
    }
}