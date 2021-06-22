#include <unordered_map>
#include "Log.h"
#include "FileReader.h"
#include "Character.h"

const std::unordered_map<CharacterClass, std::string> classNameLookup =
{
    { CHAR_CLASS_NONE,          "Classless" },
    { CHAR_CLASS_SOLDIER,       "Soldier" },
    { CHAR_CLASS_DEMOLITIONIST, "Demolitionist" },
    { CHAR_CLASS_OCCULTIST,     "Occultist" },
    { CHAR_CLASS_NIGHTBLADE,    "Nightblade" },
    { CHAR_CLASS_ARCANIST,      "Arcanist" },
    { CHAR_CLASS_SHAMAN,        "Shaman" },
    { CHAR_CLASS_INQUISITOR,    "Inquisitor" },
    { CHAR_CLASS_NECROMANCER,   "Necromancer" },
    { CHAR_CLASS_OATHKEEPER,    "Oathkeeper" },
    { CHAR_CLASS_COMMANDO,      "Commando" },
    { CHAR_CLASS_WITCHBLADE,    "Witchblade" },
    { CHAR_CLASS_BLADEMASTER,   "Blademaster" },
    { CHAR_CLASS_BATTLEMAGE,    "Battlemage" },
    { CHAR_CLASS_WARDER,        "Warder" },
    { CHAR_CLASS_TACTICIAN,     "Tactician" },
    { CHAR_CLASS_DEATH_KNIGHT,  "Death Knight" },
    { CHAR_CLASS_WARLORD,       "Warlord" },
    { CHAR_CLASS_PYROMANCER,    "Pyromancer" },
    { CHAR_CLASS_SABOTEUR,      "Saboteur" },
    { CHAR_CLASS_SORCERER,      "Sorcerer" },
    { CHAR_CLASS_ELEMENTALIST,  "Elementalist" },
    { CHAR_CLASS_PURIFIER,      "Purifier" },
    { CHAR_CLASS_DEFILER,       "Defiler" },
    { CHAR_CLASS_SHIELDBREAKER, "Shieldbreaker" },
    { CHAR_CLASS_WITCH_HUNTER,  "Witch Hunter" },
    { CHAR_CLASS_WARLOCK,       "Warlock" },
    { CHAR_CLASS_CONJURER,      "Conjurer" },
    { CHAR_CLASS_DECEIVER,      "Deceiver" },
    { CHAR_CLASS_CABALIST,      "Cabalist" },
    { CHAR_CLASS_SENTINEL,      "Sentinel" },
    { CHAR_CLASS_SPELLBREAKER,  "Spellbreaker" },
    { CHAR_CLASS_TRICKSTER,     "Trickster" },
    { CHAR_CLASS_INFILTRATOR,   "Infiltrator" },
    { CHAR_CLASS_REAPER,        "Reaper" },
    { CHAR_CLASS_DERVISH,       "Dervish" },
    { CHAR_CLASS_DRUID,         "Druid" },
    { CHAR_CLASS_MAGE_HUNTER,   "Mage Hunter" },
    { CHAR_CLASS_SPELLBINDER,   "Spellbinder" },
    { CHAR_CLASS_TEMPLAR,       "Templar" },
    { CHAR_CLASS_VINDICATOR,    "Vindicator" },
    { CHAR_CLASS_RITUALIST,     "Ritualist" },
    { CHAR_CLASS_ARCHON,        "Archon" },
    { CHAR_CLASS_APOSTATE,      "Apostate" },
    { CHAR_CLASS_PALADIN,       "Paladin" },
    { CHAR_CLASS_OPPRESSOR,     "Oppressor" },
};

bool Character::ReadFromFile(const std::filesystem::path& path)
{
    if (!IsValid() && std::filesystem::is_regular_file(path))
    {
        std::shared_ptr<EncodedFileReader> readerPtr = EncodedFileReader::Open(path.c_str());
        if (!readerPtr)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", path.string().c_str());
            return false;
        }

        EncodedFileReader* reader = readerPtr.get();
        try
        {
            ReadHeaderBlock(reader);
            ReadInfoBlock(reader);
            ReadAttributesBlock(reader);
            ReadInventoryBlock(reader);
            ReadStashBlock(reader);
            ReadRespawnBlock(reader);
            ReadWaypointBlock(reader);
            ReadMarkerBlock(reader);
            ReadShrineBlock(reader);
            ReadSkillBlock(reader);
            ReadNotesBlock(reader);
            ReadFactionBlock(reader);
            ReadUIBlock(reader);
            ReadTutorialBlock(reader);
            ReadStatsBlock(reader);

            SetState(true);
            return true;
        }
        catch (std::runtime_error&)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character file \"%\"", path.string().c_str());
            return false;
        }
    }
    return false;
}

void Character::ReadHeaderBlock(EncodedFileReader* reader)
{
    uint32_t signature = reader->ReadInt32();
    uint32_t fileVersion = reader->ReadInt32();
    if ((signature != 1480803399) || (fileVersion != 2))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file signature or version is invalid"));

    _headerBlock._charName = reader->ReadWideString();
    _headerBlock._charSex = reader->ReadInt8();

    std::string charClassName = reader->ReadString();

    int class1, class2 = 0;
    int numClasses = sscanf_s(charClassName.c_str(), "tagSkillClassName%02d%02d", &class1, &class2);
    if (numClasses == 1)
    {
        _headerBlock._charClass = (CharacterClass)class1;
    }
    else if (numClasses == 2)
    {
        _headerBlock._charClass = (CharacterClass)((class1 << 4) | class2);
    }
    else if (charClassName.empty())
    {
        _headerBlock._charClass = CHAR_CLASS_NONE;
    }

    if (classNameLookup.count(_headerBlock._charClass) == 0)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Unrecognized character class name: %", charClassName));

    _headerBlock._charLevel = reader->ReadInt32();
    _headerBlock._charIsHardcore = reader->ReadInt8();

    if (fileVersion >= 2)
    {
        _headerBlock._charExpansions = reader->ReadInt8();
        if (_headerBlock._charExpansions != 3)
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file does not have the correct expansion status (requires AoM and FG)"));
    }

    _headerBlock.ReadBlockStart(reader, GD_DATA_BLOCK_READ_VERSION);
    _headerBlock._charUID = UID16(reader);
    _headerBlock.SetState(true);
}

void Character::ReadInfoBlock(EncodedFileReader* reader)
{
    _infoBlock.ReadBlockStart(reader);

    _infoBlock._charIsModded = (reader->ReadInt8() == 0);    // For some reason 1 = not modded and 0 = modded
    _infoBlock._charIsInGame = reader->ReadInt8();           // I think this indicates whether the character has already gone through the intro cinematics?
    _infoBlock._charDifficulty = reader->ReadInt8();         // This is different for modded characters though, e.g. 130 for my S2 mod characters on Ultimate... maybe mod flag sets the 128 bit?
    _infoBlock._charMaxDifficulty = reader->ReadInt8();
    _infoBlock._charMoney = reader->ReadInt32();

    uint32_t blockVersion = _infoBlock.GetBlockVersion();
    if (blockVersion >= 4)
    {
        _infoBlock._charCrucibleDifficulty = reader->ReadInt8();
        _infoBlock._charCrucibleTributes = reader->ReadInt32();
    }
    else
    {
        _infoBlock._charCrucibleDifficulty = 0;
        _infoBlock._charCrucibleTributes = 0;
    }

    _infoBlock._charCompassState = reader->ReadInt8();

    if ((blockVersion == 3) || (blockVersion == 4))
    {
        _infoBlock._charLootMode = reader->ReadInt32();
    }
    else
    {
        _infoBlock._charLootMode = 0;
    }

    _infoBlock._charSkillWindowHelp = reader->ReadInt8();
    _infoBlock._charAlternateConfig = reader->ReadInt8();
    _infoBlock._charIsAlternateConfigEnabled = reader->ReadInt8();
    _infoBlock._charTexture = reader->ReadString();
    if (blockVersion >= 5)
    {
        uint32_t numLootFilters = reader->ReadInt32();
        for (uint32_t i = 0; i < numLootFilters; ++i)
        {
            _infoBlock._charLootFilters.push_back(reader->ReadInt8());
        }
    }

    _infoBlock.ReadBlockEnd(reader);
    _infoBlock.SetState(true);
}

void Character::ReadAttributesBlock(EncodedFileReader* reader)
{
    _attributesBlock.ReadBlockStart(reader);

    // Not sure why this is stored both here and in the header block as well
    _attributesBlock._charLevel = reader->ReadInt32();
    if (_attributesBlock._charLevel != _headerBlock._charLevel)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The character level in the attributes block (%) does not match the header data (%)", _attributesBlock._charLevel, _headerBlock._charLevel));

    _attributesBlock._charExperience = reader->ReadInt32();
    _attributesBlock._charAttributePoints = reader->ReadInt32();
    _attributesBlock._charSkillPoints = reader->ReadInt32();
    _attributesBlock._charDevotionPoints = reader->ReadInt32();
    _attributesBlock._charTotalDevotionPoints = reader->ReadInt32();
    _attributesBlock._charPhysique = reader->ReadFloat();
    _attributesBlock._charCunning = reader->ReadFloat();
    _attributesBlock._charSpirit = reader->ReadFloat();
    _attributesBlock._charHealth = reader->ReadFloat();
    _attributesBlock._charEnergy = reader->ReadFloat();

    _attributesBlock.ReadBlockEnd(reader);
    _attributesBlock.SetState(true);
}

void Character::ReadInventoryBlock(EncodedFileReader* reader)
{
    _inventoryBlock.ReadBlockStart(reader);

    if (reader->ReadInt8() != 0)
    {
        // Character inventory
        uint32_t numTabs = reader->ReadInt32();
        _inventoryBlock._charInventory.SetFocusedTab(reader->ReadInt32());
        _inventoryBlock._charInventory.SetSelectedTab(reader->ReadInt32());
        for (uint32_t i = 0; i < numTabs; ++i)
        {
            _inventoryBlock._charInventory.ReadStashTab(reader);
        }

        // Character equipped items
        _inventoryBlock._charEquipped.SetActiveWeaponSet(reader->ReadInt8());
        for (uint32_t i = 0; i < MAX_CHAR_INV_SLOT; ++i)
        {
            // This seems kind of redundant since we already read the active weapon set earlier
            if ((i == CHAR_INV_SLOT_MAIN_1) || (i == CHAR_INV_SLOT_MAIN_2))
            {
                bool isWeaponSetActive = (reader->ReadInt8() != 0);
            }

            std::shared_ptr<Item> item = std::make_shared<Item>(reader);
            _inventoryBlock._charEquipped.AddItem(item, i, 0);
            _inventoryBlock._charEquipped.SetAttachState(i, reader->ReadInt8());
        }
    }

    _inventoryBlock.ReadBlockEnd(reader);
    _inventoryBlock.SetState(true);
}

void Character::ReadStashBlock(EncodedFileReader* reader)
{
    _stashBlock.ReadBlockStart(reader);

    uint32_t numTabs = reader->ReadInt32();
    for (uint32_t i = 0; i < numTabs; ++i)
    {
        _stashBlock._charStash.ReadStashTab(reader);
    }

    _stashBlock._charStash.SetHardcore(_headerBlock._charIsHardcore);

    _stashBlock.ReadBlockEnd(reader);
    _stashBlock.SetState(true);
}

void Character::ReadRespawnBlock(EncodedFileReader* reader)
{
    _respawnBlock.ReadBlockStart(reader);

    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _respawnBlock._charRespawnsNormal.emplace_back(reader);
    }

    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _respawnBlock._charRespawnsElite.emplace_back(reader);
    }

    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _respawnBlock._charRespawnsUltimate.emplace_back(reader);
    }

    _respawnBlock._charCurrentRespawnNormal = UID16(reader);
    _respawnBlock._charCurrentRespawnElite = UID16(reader);
    _respawnBlock._charCurrentRespawnUltimate = UID16(reader);

    _respawnBlock.ReadBlockEnd(reader);
    _respawnBlock.SetState(true);
}

void Character::ReadWaypointBlock(EncodedFileReader* reader)
{
    _waypointBlock.ReadBlockStart(reader);

    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _waypointBlock._charWaypointsNormal.emplace_back(reader);
    }

    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _waypointBlock._charWaypointsElite.emplace_back(reader);
    }

    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _waypointBlock._charWaypointsUltimate.emplace_back(reader);
    }

    _waypointBlock.ReadBlockEnd(reader);
    _waypointBlock.SetState(true);
}

void Character::ReadMarkerBlock(EncodedFileReader* reader)
{
    _markerBlock.ReadBlockStart(reader);

    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _markerBlock._charMarkersNormal.emplace_back(reader);
    }

    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _markerBlock._charMarkersElite.emplace_back(reader);
    }

    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _markerBlock._charMarkersUltimate.emplace_back(reader);
    }

    _markerBlock.ReadBlockEnd(reader);
    _markerBlock.SetState(true);
}

void Character::ReadShrineBlock(EncodedFileReader* reader)
{
    _shrineBlock.ReadBlockStart(reader);

    for (uint32_t i = 0; i < 6; ++i)
    {
        uint32_t numIDs = reader->ReadInt32();
        for (uint32_t j = 0; j < numIDs; ++j)
        {
            _shrineBlock._charShrines[i].emplace_back(reader);
        }
    }

    _shrineBlock.ReadBlockEnd(reader);
    _shrineBlock.SetState(true);
}

void Character::ReadSkillBlock(EncodedFileReader* reader)
{
    _skillBlock.ReadBlockStart(reader);

    uint32_t numSkills = reader->ReadInt32();
    for (uint32_t i = 0; i < numSkills; ++i)
    {
        _skillBlock._charClassSkills.emplace_back(reader);
    }

    _skillBlock._charMasteriesAllowed = reader->ReadInt32();
    _skillBlock._charSkillReclaimed = reader->ReadInt32();
    _skillBlock._charDevotionReclaimed = reader->ReadInt32();

    numSkills = reader->ReadInt32();
    for (uint32_t i = 0; i < numSkills; ++i)
    {
        _skillBlock._charItemSkills.emplace_back(reader);
    }

    _skillBlock.ReadBlockEnd(reader);
    _skillBlock.SetState(true);
}

void Character::ReadNotesBlock(EncodedFileReader* reader)
{
    _notesBlock.ReadBlockStart(reader);

    uint32_t numNotes = reader->ReadInt32();
    for (uint32_t i = 0; i < numNotes; ++i)
    {
        _notesBlock._charNotes.push_back(reader->ReadString());
    }

    _notesBlock.ReadBlockEnd(reader);
    _notesBlock.SetState(true);
}

void Character::ReadFactionBlock(EncodedFileReader* reader)
{
    _factionBlock.ReadBlockStart(reader);

    _factionBlock._unk1 = reader->ReadInt32();

    uint32_t numFactions = reader->ReadInt32();
    for (uint32_t i = 0; i < numFactions; ++i)
    {
        _factionBlock._charFactions.emplace_back(reader);
        _factionBlock._charFactions[i]._factionID = static_cast<Factions>(i);
    }

    _factionBlock.ReadBlockEnd(reader);
    _factionBlock.SetState(true);
}

void Character::ReadUIBlock(EncodedFileReader* reader)
{
    _UIBlock.ReadBlockStart(reader);

    _UIBlock._unk1 = reader->ReadInt8();
    _UIBlock._unk2 = reader->ReadInt32();
    _UIBlock._unk3 = reader->ReadInt8();

    // Is this always hardcoded at 5? Not sure... it would probably help to know what this actually does
    for (uint32_t i = 0; i < 5; ++i)
    {
        CharacterUIBlock::CharacterUIUnkData unknown;
        unknown._unk1 = reader->ReadString();
        unknown._unk2 = reader->ReadString();
        unknown._unk3 = reader->ReadInt8();
        unknown.SetState(true);
        _UIBlock._unk4.push_back(unknown);
    }

    uint32_t numSlots = (_UIBlock.GetBlockVersion() >= 5) ? 46 : 36;
    for (uint32_t i = 0; i < numSlots; ++i)
    {
        CharacterUIBlock::CharacterUISlot slot;
        memset(&slot, 0, sizeof(CharacterUIBlock::CharacterUISlot));

        slot._slotType = reader->ReadInt32();
        switch (slot._slotType)
        {
            case 0:    // Item/Class Skill
                slot._slotSkillName = reader->ReadString();
                slot._slotIsItemSkill = reader->ReadInt8();
                slot._slotItemName = reader->ReadString();
                slot._slotEquip = reader->ReadInt32();
                break;
            case 4:    // Item
                slot._slotItemName = reader->ReadString();
                slot._slotBitmapUp = reader->ReadString();
                slot._slotBitmapDown = reader->ReadString();
                slot._slotLabel = reader->ReadWideString();
                break;
            // Not sure what these are; they appear to contain no data other than the slot type
            case 2:
            case 3:
            case -1:    // Maybe an empty slot?
                break;
            default:
                throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_WARN, "Invalid or unsupported item slot type \"%\"", slot._slotType));
        }
        slot.SetState(true);
        _UIBlock._charUISlots.push_back(slot);
    }

    _UIBlock._charCameraDistance = reader->ReadFloat();

    _UIBlock.ReadBlockEnd(reader);
    _UIBlock.SetState(true);
}

void Character::ReadTutorialBlock(EncodedFileReader* reader)
{
    _tutorialBlock.ReadBlockStart(reader);

    uint32_t numTutorials = reader->ReadInt32();
    for (uint32_t i = 0; i < numTutorials; ++i)
    {
        _tutorialBlock._charTutorials.push_back(reader->ReadInt32());
    }

    _tutorialBlock.ReadBlockEnd(reader);
    _tutorialBlock.SetState(true);
}

void Character::ReadStatsBlock(EncodedFileReader* reader)
{
    _statsBlock.ReadBlockStart(reader);

    _statsBlock._charPlayTime = reader->ReadInt32();
    _statsBlock._charDeaths = reader->ReadInt32();
    _statsBlock._charKills = reader->ReadInt32();
    _statsBlock._charExpFromKills = reader->ReadInt32();
    _statsBlock._charHealthPotsUsed = reader->ReadInt32();
    _statsBlock._charManaPotsUsed = reader->ReadInt32();
    _statsBlock._charMaxLevel = reader->ReadInt32();
    _statsBlock._charHitsReceived = reader->ReadInt32();
    _statsBlock._charHitsInflicted = reader->ReadInt32();
    _statsBlock._charCritsInflicted = reader->ReadInt32();
    _statsBlock._charCritsReceived = reader->ReadInt32();
    _statsBlock._charGreatestDamageInflicted = reader->ReadFloat();

    for (uint32_t i = 0; i < 3; ++i)
    {
        _statsBlock._charDifficultyStats[i]._difficulty = i;
        _statsBlock._charDifficultyStats[i]._greatestEnemyKilled = reader->ReadString();
        _statsBlock._charDifficultyStats[i]._greatestEnemyLevel = reader->ReadInt32();
        _statsBlock._charDifficultyStats[i]._greatestEnemyHealth = reader->ReadInt32();
        _statsBlock._charDifficultyStats[i]._lastAttacked = reader->ReadString();
        _statsBlock._charDifficultyStats[i]._lastAttackedBy = reader->ReadString();
        _statsBlock._charDifficultyStats[i].SetState(true);
    }

    _statsBlock._charChampionKills = reader->ReadInt32();
    _statsBlock._charLastAttackedDA = reader->ReadFloat();
    _statsBlock._charLastAttackedByOA = reader->ReadFloat();
    _statsBlock._charGreatestDamageReceived = reader->ReadFloat();
    _statsBlock._charHeroKills = reader->ReadInt32();
    _statsBlock._charItemsCrafted = reader->ReadInt32();
    _statsBlock._charRelicsCrafted = reader->ReadInt32();
    _statsBlock._charTranscendentRelicsCrafted = reader->ReadInt32();
    _statsBlock._charMythicalRelicsCrafted = reader->ReadInt32();
    _statsBlock._charShrinesRestored = reader->ReadInt32();
    _statsBlock._charOneShotChestsOpened = reader->ReadInt32();
    _statsBlock._charLoreNotesCollected = reader->ReadInt32();

    for (uint32_t i = 0; i < 3; ++i)
    {
        _statsBlock._charDifficultyStats[i]._nemesisKills = reader->ReadInt32();
    }

    if (_statsBlock.GetBlockVersion() >= 9)
    {
        _statsBlock._charCrucibleGreatestWave = reader->ReadInt32();
        _statsBlock._charCrucibleGreatestScore = reader->ReadInt32();
        _statsBlock._charCrucibleDefensesBuilt = reader->ReadInt32();
        _statsBlock._charCrucibleBuffsUsed = reader->ReadInt32();
    }
    else
    {
        _statsBlock._charCrucibleGreatestWave = 0;
        _statsBlock._charCrucibleGreatestScore = 0;
        _statsBlock._charCrucibleDefensesBuilt = 0;
        _statsBlock._charCrucibleBuffsUsed = 0;
    }

    if (_statsBlock.GetBlockVersion() >= 11)
    {
        uint32_t numEntries = reader->ReadInt32();
        for (uint32_t i = 0; i < numEntries; ++i)
        {
            std::string name = reader->ReadString();
            _statsBlock._charSRShrinesUsed[name] = reader->ReadInt32();
        }

        _statsBlock._charSRSoulsCollected = reader->ReadInt32();
        _statsBlock._charSRFlag = reader->ReadInt32();
        _statsBlock._charMeritUsed = reader->ReadInt8();
    }
    else
    {
        _statsBlock._charSRSoulsCollected = 0;
        _statsBlock._charSRFlag = 0;
        _statsBlock._charMeritUsed = 0;
    }

    _statsBlock._unk1 = reader->ReadInt32();
    _statsBlock._unk2 = reader->ReadInt32();

    _statsBlock.ReadBlockEnd(reader);
    _statsBlock.SetState(true);
}