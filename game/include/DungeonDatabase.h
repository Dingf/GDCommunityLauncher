#ifndef INC_GDCL_GAME_DUNGEON_DATABASE_H
#define INC_GDCL_GAME_DUNGEON_DATABASE_H

#include <map>
#include <string>

class DungeonDatabase
{
    public:
        struct DungeonDBEntry
        {
            bool     _active;
            uint32_t _level;
        };

        typedef void (__thiscall* DungeonDBUpdateCallback)(std::map<std::string, DungeonDBEntry>&);

        static DungeonDatabase& GetInstance();

        bool IsLoaded() const { return ((!_zoneData.empty()) && (!_dungeonData.empty())); }

        bool IsDungeonZone(const std::string& zone) const { return ((_zoneData.count(zone) > 0) && (_dungeonData.count(_zoneData.at(zone)) > 0)); }

        const DungeonDBEntry& GetEntryByName(const std::string& name) const { return _dungeonData.at(name); }
        const DungeonDBEntry& GetEntryByZone(const std::string& zone) const { return _dungeonData.at(_zoneData.at(zone)); }

        void Load(const char* buffer, size_t size, DungeonDBUpdateCallback callback);

        void Update() { if (_callback != nullptr) _callback(_dungeonData); }

    private:
        DungeonDatabase() { _callback = nullptr; }
        DungeonDatabase(DungeonDatabase&) = delete;
        void operator=(const DungeonDatabase&) = delete;

        DungeonDBUpdateCallback _callback;
        std::map<std::string, std::string> _zoneData;
        std::map<std::string, DungeonDBEntry> _dungeonData;
};

#endif//INC_GDCL_GAME_DUNGEON_DATABASE_H