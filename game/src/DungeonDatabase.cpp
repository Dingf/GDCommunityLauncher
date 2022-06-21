#include <sstream>
#include <iomanip>
#include "DungeonDatabase.h"

DungeonDatabase& DungeonDatabase::GetInstance()
{
    static DungeonDatabase instance;
    return instance;
}

void DungeonDatabase::Load(const char* buffer, size_t size, DungeonDBUpdateCallback callback)
{
    if ((_zoneData.empty()) && (_dungeonData.empty()) && (_callback == nullptr))
    {
        std::string buffer(buffer, size);
        std::stringstream ss(buffer);

        std::string zone;
        std::string name;
        while (ss >> std::quoted(zone, '"', '\0') >> std::quoted(name, '"', '\0'))
        {
            _zoneData[zone] = name;
            if (_dungeonData.count(name) == 0)
            {
                _dungeonData[name]._active = false;
                _dungeonData[name]._level = 0;
            }
        }

        _callback = callback;
    }
}