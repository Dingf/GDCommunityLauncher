#ifndef INC_GDCL_GAME_CHARACTER_H
#define INC_GDCL_GAME_CHARACTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "FileReader.h"
#include "JSONObject.h"
#include "GDDataBlock.h"
#include "UID.h"
#include "ItemContainer.h"
#include "Skill.h"
#include "Stash.h"
#include "Faction.h"

enum CharacterClass
{
    CHAR_CLASS_NONE = 0x00,
    CHAR_CLASS_SOLDIER = 0x01,
    CHAR_CLASS_DEMOLITIONIST = 0x02,
    CHAR_CLASS_OCCULTIST = 0x03,
    CHAR_CLASS_NIGHTBLADE = 0x04,
    CHAR_CLASS_ARCANIST = 0x05,
    CHAR_CLASS_SHAMAN = 0x06,
    CHAR_CLASS_INQUISITOR = 0x07,
    CHAR_CLASS_NECROMANCER = 0x08,
    CHAR_CLASS_OATHKEEPER = 0x09,
    CHAR_CLASS_COMMANDO = 0x12,
    CHAR_CLASS_WITCHBLADE = 0x13,
    CHAR_CLASS_BLADEMASTER = 0x14,
    CHAR_CLASS_BATTLEMAGE = 0x15,
    CHAR_CLASS_WARDER = 0x16,
    CHAR_CLASS_TACTICIAN = 0x17,
    CHAR_CLASS_DEATH_KNIGHT = 0x18,
    CHAR_CLASS_WARLORD = 0x19,
    CHAR_CLASS_PYROMANCER = 0x23,
    CHAR_CLASS_SABOTEUR = 0x24,
    CHAR_CLASS_SORCERER = 0x25,
    CHAR_CLASS_ELEMENTALIST = 0x26,
    CHAR_CLASS_PURIFIER = 0x27,
    CHAR_CLASS_DEFILER = 0x28,
    CHAR_CLASS_SHIELDBREAKER = 0x29,
    CHAR_CLASS_WITCH_HUNTER = 0x34,
    CHAR_CLASS_WARLOCK = 0x35,
    CHAR_CLASS_CONJURER = 0x36,
    CHAR_CLASS_DECEIVER = 0x37,
    CHAR_CLASS_CABALIST = 0x38,
    CHAR_CLASS_SENTINEL = 0x39,
    CHAR_CLASS_SPELLBREAKER = 0x45,
    CHAR_CLASS_TRICKSTER = 0x46,
    CHAR_CLASS_INFILTRATOR = 0x47,
    CHAR_CLASS_REAPER = 0x48,
    CHAR_CLASS_DERVISH = 0x49,
    CHAR_CLASS_DRUID = 0x56,
    CHAR_CLASS_MAGE_HUNTER = 0x57,
    CHAR_CLASS_SPELLBINDER = 0x58,
    CHAR_CLASS_TEMPLAR = 0x59,
    CHAR_CLASS_VINDICATOR = 0x67,
    CHAR_CLASS_RITUALIST = 0x68,
    CHAR_CLASS_ARCHON = 0x69,
    CHAR_CLASS_APOSTATE = 0x78,
    CHAR_CLASS_PALADIN = 0x79,
    CHAR_CLASS_OPPRESSOR = 0x89,
};

enum CharacterInventorySlot
{
    CHAR_INV_SLOT_HEAD = 0,
    CHAR_INV_SLOT_NECK = 1,
    CHAR_INV_SLOT_CHEST = 2,
    CHAR_INV_SLOT_LEGS = 3,
    CHAR_INV_SLOT_FEET = 4,
    CHAR_INV_SLOT_HANDS = 5,
    CHAR_INV_SLOT_L_RING = 6,
    CHAR_INV_SLOT_R_RING = 7,
    CHAR_INV_SLOT_WAIST = 8,
    CHAR_INV_SLOT_SHOULDER = 9,
    CHAR_INV_SLOT_MEDAL = 10,
    CHAR_INV_SLOT_RELIC = 11,
    CHAR_INV_SLOT_MAIN_1 = 12,
    CHAR_INV_SLOT_OFF_1 = 13,
    CHAR_INV_SLOT_MAIN_2 = 14,
    CHAR_INV_SLOT_OFF_2 = 15,
    MAX_CHAR_INV_SLOT = 16,
};

class Character : public JSONObject
{
    public:
        Character() {}
        Character(const std::filesystem::path& path) { ReadFromFile(path); }

        bool ReadFromFile(const std::filesystem::path& path);
        bool ReadFromBuffer(uint8_t* data, size_t size);

        web::json::value ToJSON() const;

    private:
        void ReadHeaderBlock(EncodedFileReader* reader);
        void ReadInfoBlock(EncodedFileReader* reader);
        void ReadAttributesBlock(EncodedFileReader* reader);
        void ReadInventoryBlock(EncodedFileReader* reader);
        void ReadStashBlock(EncodedFileReader* reader);
        void ReadRespawnBlock(EncodedFileReader* reader);
        void ReadWaypointBlock(EncodedFileReader* reader);
        void ReadMarkerBlock(EncodedFileReader* reader);
        void ReadShrineBlock(EncodedFileReader* reader);
        void ReadSkillBlock(EncodedFileReader* reader);
        void ReadNotesBlock(EncodedFileReader* reader);
        void ReadFactionBlock(EncodedFileReader* reader);
        void ReadUIBlock(EncodedFileReader* reader);
        void ReadTutorialBlock(EncodedFileReader* reader);
        void ReadStatsBlock(EncodedFileReader* reader);

        // Header block, ID = 0, Version = 6,7,8
        struct CharacterHeaderBlock : public GDDataBlock
        {
            CharacterHeaderBlock() : GDDataBlock(0x00, 0xE0) {}

            web::json::value ToJSON() const;

            std::wstring   _charName;
            uint8_t        _charSex;
            CharacterClass _charClass;
            uint32_t       _charLevel;
            uint8_t        _charIsHardcore;
            uint8_t        _charExpansions;
            UID16          _charUID;
        }
        _headerBlock;

        // Info block, ID = 1, Version = 3,4,5
        struct CharacterInfoBlock : public GDDataBlock
        {
            CharacterInfoBlock() : GDDataBlock(0x01, 0x1C) {}

            web::json::value ToJSON() const;

            uint8_t        _charIsModded;
            uint8_t        _charIsInGame;
            uint8_t        _charDifficulty;
            uint8_t        _charMaxDifficulty;
            uint32_t       _charMoney;
            uint8_t        _charCrucibleDifficulty;
            uint32_t       _charCrucibleTributes;
            uint8_t        _charCompassState;
            uint32_t       _charLootMode;
            uint8_t        _charSkillWindowHelp;
            uint8_t        _charAlternateConfig;
            uint8_t        _charIsAlternateConfigEnabled;
            std::string    _charTexture;
            std::vector<uint8_t>  _charLootFilters;
        }
        _infoBlock;

        // Attributes Block, ID = 2, Version = 8
        struct CharacterAttributeBlock : public GDDataBlock
        {
            CharacterAttributeBlock() : GDDataBlock(0x02, 0x80) {}

            web::json::value ToJSON() const;

            uint32_t       _charLevel;
            uint32_t       _charExperience;
            uint32_t       _charAttributePoints;
            uint32_t       _charSkillPoints;
            uint32_t       _charDevotionPoints;
            uint32_t       _charTotalDevotionPoints;
            float          _charPhysique;
            float          _charCunning;
            float          _charSpirit;
            float          _charHealth;
            float          _charEnergy;
        }
        _attributesBlock;

        // Inventory Block, ID = 3, Version = 4
        struct CharacterInventoryBlock : public GDDataBlock
        {
            CharacterInventoryBlock() : GDDataBlock(0x03, 0x08) {}

            web::json::value ToJSON() const;

            class CharacterInventory : public Stash
            {
                public:
                    ItemContainerType GetContainerType() const { return ITEM_CONTAINER_CHAR_BAG; }

                    size_t GetBufferSize() const;

                    void Read(EncodedFileReader* reader);
                    void Write(EncodedFileWriter* writer);

                    uint32_t GetFocusedTab() const { return _focusedTab; }
                    uint32_t GetSelectedTab() const { return _selectedTab; }

                    void SetFocusedTab(uint32_t tab) { _focusedTab = tab; }
                    void SetSelectedTab(uint32_t tab) { _selectedTab = tab; }

                private:
                    uint32_t _focusedTab;
                    uint32_t _selectedTab;
            }
            _charInventory;

            class CharacterEquipped : public ItemContainer
            {
                public:
                    CharacterEquipped() : ItemContainer(1, MAX_CHAR_INV_SLOT) {}

                    size_t GetBufferSize() const;

                    void Read(EncodedFileReader* reader);
                    void Write(EncodedFileWriter* writer);

                    ItemContainerType GetContainerType() const { return ITEM_CONTAINER_CHAR_INVENTORY; }

                    bool IsUsingSecondaryWeaponSet() const { return _activeWeaponSet; }
                    bool GetAttachState(uint32_t index) const { return (index < MAX_CHAR_INV_SLOT) ? _attached[index] : false; }

                    void SetActiveWeaponSet(bool secondary) { _activeWeaponSet = secondary; }
                    void SetAttachState(uint32_t index, bool state) { if (index < MAX_CHAR_INV_SLOT) { _attached[index] = state; } }

                private:
                    bool _activeWeaponSet;
                    bool _attached[MAX_CHAR_INV_SLOT];
                    int8_t _weaponSet1;
                    int8_t _weaponSet2;
            }
            _charEquipped;

        }
        _inventoryBlock;

        // Stash Block, ID = 4, Version = 5,6
        struct CharacterStashBlock : public GDDataBlock
        {
            CharacterStashBlock() : GDDataBlock(0x04, 0x30) {}

            web::json::value ToJSON() const;

            class CharacterStash : public Stash
            {
                public:
                    ItemContainerType GetContainerType() const { return ITEM_CONTAINER_CHAR_STASH; }

                    void Read(EncodedFileReader* reader);
                    void Write(EncodedFileWriter* writer);
            }
            _charStash;
        }
        _stashBlock;

        // Respawn Block, ID = 5, Version = 1
        struct CharacterRespawnBlock : public GDDataBlock
        {
            CharacterRespawnBlock() : GDDataBlock(0x05, 0x01) {}

            web::json::value ToJSON() const;

            std::vector<UID16> _charRespawnsNormal;
            std::vector<UID16> _charRespawnsElite;
            std::vector<UID16> _charRespawnsUltimate;
            UID16              _charCurrentRespawnNormal;
            UID16              _charCurrentRespawnElite;
            UID16              _charCurrentRespawnUltimate;
        }
        _respawnBlock;

        // Waypoints Block, ID = 6, Version = 1
        struct CharacterWaypointBlock : public GDDataBlock
        {
            CharacterWaypointBlock() : GDDataBlock(0x06, 0x01) {}

            web::json::value ToJSON() const;

            std::vector<UID16> _charWaypointsNormal;
            std::vector<UID16> _charWaypointsElite;
            std::vector<UID16> _charWaypointsUltimate;
        }
        _waypointBlock;

        // Markers Block, ID = 7, Version = 1
        struct CharacterMarkerBlock : public GDDataBlock
        {
            CharacterMarkerBlock() : GDDataBlock(0x07, 0x01) {}

            web::json::value ToJSON() const;

            std::vector<UID16> _charMarkersNormal;
            std::vector<UID16> _charMarkersElite;
            std::vector<UID16> _charMarkersUltimate;
        }
        _markerBlock;

        // Shrines Block, ID = 17, Version = 2
        struct CharacterShrineBlock : public GDDataBlock
        {
            CharacterShrineBlock() : GDDataBlock(0x11, 0x02) {}

            web::json::value ToJSON() const;

            // Two lists per difficulty; first is for restored, second is for discovered
            std::vector<UID16> _charShrines[6];
        }
        _shrineBlock;

        // Skills Block, ID = 8, Version = 5,6
        struct CharacterSkillBlock : public GDDataBlock
        {
            CharacterSkillBlock() : GDDataBlock(0x08, 0x30) {}

            web::json::value ToJSON() const;

            uint32_t           _charMasteriesAllowed;    // Always 2?
            uint32_t           _charSkillReclaimed;
            uint32_t           _charDevotionReclaimed;
            std::vector<ClassSkill> _charClassSkills;
            std::vector<ItemSkill>  _charItemSkills;
            uint32_t           _unk1;
        }
        _skillBlock;

        // Notes Block, ID = 12, Version = 1
        struct CharacterNotesBlock : public GDDataBlock
        {
            CharacterNotesBlock() : GDDataBlock(0x0C, 0x01) {}

            web::json::value ToJSON() const;

            std::vector<std::string> _charNotes;
        }
        _notesBlock;

        // Factions Block, ID = 13, Version = 5
        struct CharacterFactionBlock : public GDDataBlock
        {
            CharacterFactionBlock() : GDDataBlock(0x0D, 0x10) {}

            web::json::value ToJSON() const;

            uint32_t             _unk1;             // GDStash has this listed as "faction", not sure what that means... value appears to always be 0
            std::vector<Faction> _charFactions;
        }
        _factionBlock;

        // UI Settings Block, ID = 14, Version = 4,5,6,7
        struct CharacterUIBlock : public GDDataBlock
        {
            CharacterUIBlock() : GDDataBlock(0x0E, 0x78) {}

            web::json::value ToJSON() const;

            struct CharacterUIUnkData : public JSONObject
            {
                web::json::value ToJSON() const;

                std::string    _unk1;
                std::string    _unk2;
                uint8_t        _unk3;
            };

            struct CharacterUISlot : public JSONObject
            {
                web::json::value ToJSON() const;

                int32_t        _slotType;           // 0 = item/class skill, 4 = item
                std::string    _slotSkillName;
                std::string    _slotItemName;       // For skills, this will be the item that grants the skill (if applicable)
                uint8_t        _slotIsItemSkill;
                uint32_t       _slotEquip;
                std::string    _slotBitmapUp;
                std::string    _slotBitmapDown;
                std::wstring   _slotLabel;
            };

            uint8_t            _unk1;
            uint32_t           _unk2;
            uint8_t            _unk3;
            uint32_t           _unk5;
            int32_t            _unk6;
            float              _charCameraDistance;
            std::vector<CharacterUIUnkData> _unk4;
            std::vector<CharacterUISlot>    _charUISlots;
        }
        _UIBlock;

        // Tutorial Block, ID = 15, Version = 1
        struct CharacterTutorialBlock : public GDDataBlock
        {
            CharacterTutorialBlock() : GDDataBlock(0x0F, 0x01) {}

            web::json::value ToJSON() const;

            std::vector<uint32_t> _charTutorials;
        }
        _tutorialBlock;

        // Stats Block, ID = 16, Version = 7,9,11
        struct CharacterStatsBlock : public GDDataBlock
        {
            CharacterStatsBlock() : GDDataBlock(0x10, 0x540) {}

            web::json::value ToJSON() const;

            struct CharacterPerDifficultyStats : public JSONObject
            {
                web::json::value ToJSON() const;

                uint32_t    _difficulty;
                std::string _greatestEnemyKilled;
                uint32_t    _greatestEnemyLevel;
                uint32_t    _greatestEnemyHealth;
                std::string _lastAttacked;
                std::string _lastAttackedBy;
                uint32_t    _nemesisKills;
            };

            uint32_t _charPlayTime;
            uint32_t _charDeaths;
            uint32_t _charKills;
            uint32_t _charExpFromKills;
            uint32_t _charHealthPotsUsed;
            uint32_t _charManaPotsUsed;
            uint32_t _charMaxLevel;
            uint32_t _charHitsReceived;
            uint32_t _charHitsInflicted;
            uint32_t _charCritsInflicted;
            uint32_t _charCritsReceived;
            uint32_t _charChampionKills;
            uint32_t _charHeroKills;
            uint32_t _charItemsCrafted;
            uint32_t _charRelicsCrafted;
            uint32_t _charTranscendentRelicsCrafted;
            uint32_t _charMythicalRelicsCrafted;
            uint32_t _charShrinesRestored;
            uint32_t _charOneShotChestsOpened;
            uint32_t _charLoreNotesCollected;
            uint32_t _unk1;
            uint32_t _unk2;

            float    _charLastAttackedDA;       // Strange that these two values aren't per difficulty, even though the last enemy attacked/attacked by stat is
            float    _charLastAttackedByOA;     // Confirmed that the values remain the same across difficulties, even though the last attacked enemies don't
            float    _charGreatestDamageInflicted;
            float    _charGreatestDamageReceived;

            // Version 9+ - Crucible Stats
            uint32_t _charCrucibleGreatestWave;
            uint32_t _charCrucibleGreatestScore;
            uint32_t _charCrucibleDefensesBuilt;
            uint32_t _charCrucibleBuffsUsed;

            // Version 11+ - SR/FG Stats
            std::unordered_map<std::string, uint32_t> _charSRShrinesUsed;      // Also lists movement skills, but I'm not sure what the value means. It's clearly not the number of times the skill has been used.
            uint32_t _charSRSoulsCollected;     // Not 100% sure about this one
            uint32_t _charSRFlag;               // Seems to either be 10 if SR has been completed or 0 otherwise. Might need to test with some lower level/difficulty characters...
            uint8_t  _charMeritUsed;            // 3 = Ultimate Merit, 0 = No Merit. Need to test Elite Merit case as well.

            CharacterPerDifficultyStats _charDifficultyStats[3];
        }
        _statsBlock;
};


#endif//INC_GDCL_GAME_CHARACTER_H