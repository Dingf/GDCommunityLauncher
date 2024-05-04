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
    if (std::filesystem::is_regular_file(path))
    {
        EncodedFileReader reader(path);
        if (!reader.HasData())
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", path.string().c_str());
            return false;
        }

        try
        {
            ReadHeaderBlock(&reader);
            ReadInfoBlock(&reader);
            ReadAttributesBlock(&reader);
            ReadInventoryBlock(&reader);
            ReadStashBlock(&reader);
            ReadRespawnBlock(&reader);
            ReadWaypointBlock(&reader);
            ReadMarkerBlock(&reader);
            ReadShrineBlock(&reader);
            ReadSkillBlock(&reader);
            ReadNotesBlock(&reader);
            ReadFactionBlock(&reader);
            ReadUIBlock(&reader);
            ReadTutorialBlock(&reader);
            ReadStatsBlock(&reader);
            return true;
        }
        catch (std::runtime_error&)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character file \"%\"", path.string().c_str());
            return false;
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "The path \"%\" is not recognized as a file.", path.string());
    }
    return false;
}

bool Character::ReadFromBuffer(uint8_t* data, size_t size)
{
    EncodedFileReader reader(data, size);
    if (!reader.HasData())
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read character data from buffer");
        return false;
    }

    try
    {
        ReadHeaderBlock(&reader);
        ReadInfoBlock(&reader);
        ReadAttributesBlock(&reader);
        ReadInventoryBlock(&reader);
        ReadStashBlock(&reader);
        ReadRespawnBlock(&reader);
        ReadWaypointBlock(&reader);
        ReadMarkerBlock(&reader);
        ReadShrineBlock(&reader);
        ReadSkillBlock(&reader);
        ReadNotesBlock(&reader);
        ReadFactionBlock(&reader);
        ReadUIBlock(&reader);
        ReadTutorialBlock(&reader);
        ReadStatsBlock(&reader);
        return true;
    }
    catch (std::runtime_error&)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data from buffer");
        return false;
    }
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
    }

    _headerBlock.ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_VERSION);
    _headerBlock._charUID = UID16(reader);
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
        _infoBlock._charLootFilters.clear();
        uint32_t numLootFilters = reader->ReadInt32();
        for (uint32_t i = 0; i < numLootFilters; ++i)
        {
            _infoBlock._charLootFilters.push_back(reader->ReadInt8());
        }
    }

    _infoBlock.ReadBlockEnd(reader);
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
}

size_t Character::CharacterInventoryBlock::CharacterInventory::GetBufferSize() const
{
    return 8 + Stash::GetBufferSize();
}

void Character::CharacterInventoryBlock::CharacterInventory::Read(EncodedFileReader* reader)
{
    uint32_t numTabs = reader->ReadInt32();
    SetFocusedTab(reader->ReadInt32());
    SetSelectedTab(reader->ReadInt32());
    ReadStashTabs(reader, numTabs);
}

void Character::CharacterInventoryBlock::CharacterInventory::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32((uint32_t)_stashTabs.size());
    writer->BufferInt32(_focusedTab);
    writer->BufferInt32(_selectedTab);
    WriteStashTabs(writer);
}

size_t Character::CharacterInventoryBlock::CharacterEquipped::GetBufferSize() const
{
    size_t size = 1;
    for (auto pair : GetItemList())
    {
        uint32_t index = (pair.second & 0xFFFFFFFF);
        if ((index == CHAR_INV_SLOT_MAIN_1) || (index == CHAR_INV_SLOT_MAIN_2))
            size++;

        size += pair.first->GetBufferSize();
        size++;
    }
    return size;
}

void Character::CharacterInventoryBlock::CharacterEquipped::Read(EncodedFileReader* reader)
{
    SetActiveWeaponSet(reader->ReadInt8());
    for (uint32_t i = 0; i < MAX_CHAR_INV_SLOT; ++i)
    {
        if (i == CHAR_INV_SLOT_MAIN_1)
            _weaponSet1 = reader->ReadInt8();
        else if (i == CHAR_INV_SLOT_MAIN_2)
            _weaponSet2 = reader->ReadInt8();

        Item item(reader);
        AddItem(item, 0, i);
        SetAttachState(i, reader->ReadInt8());
    }
}

void Character::CharacterInventoryBlock::CharacterEquipped::Write(EncodedFileWriter* writer)
{
    writer->BufferInt8(_activeWeaponSet);
    for (auto pair : GetItemList())
    {
        uint32_t index = (pair.second & 0xFFFFFFFF);
        if (index == CHAR_INV_SLOT_MAIN_1)
            writer->BufferInt8(_weaponSet1);
        else if (index == CHAR_INV_SLOT_MAIN_2)
            writer->BufferInt8(_weaponSet2);

        pair.first->Write(writer);
        writer->BufferInt8(GetAttachState(index));
    }
}

void Character::ReadInventoryBlock(EncodedFileReader* reader)
{
    _inventoryBlock.ReadBlockStart(reader);

    if (reader->ReadInt8() != 0)
    {
        _inventoryBlock._charInventory.Read(reader);
        _inventoryBlock._charEquipped.Read(reader);
    }

    _inventoryBlock.ReadBlockEnd(reader);
}

void Character::CharacterStashBlock::CharacterStash::Read(EncodedFileReader* reader)
{
    uint32_t numTabs = reader->ReadInt32();
    ReadStashTabs(reader, numTabs);
}

void Character::CharacterStashBlock::CharacterStash::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32((uint32_t)_stashTabs.size());
    WriteStashTabs(writer);
}

void Character::ReadStashBlock(EncodedFileReader* reader)
{
    _stashBlock.ReadBlockStart(reader);

    _stashBlock._charStash.Read(reader);
    _stashBlock._charStash.SetHardcore(_headerBlock._charIsHardcore);

    _stashBlock.ReadBlockEnd(reader);
}

void Character::ReadRespawnBlock(EncodedFileReader* reader)
{
    _respawnBlock.ReadBlockStart(reader);

    _respawnBlock._charRespawnsNormal.clear();
    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _respawnBlock._charRespawnsNormal.emplace_back(reader);
    }

    _respawnBlock._charRespawnsElite.clear();
    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _respawnBlock._charRespawnsElite.emplace_back(reader);
    }

    _respawnBlock._charRespawnsUltimate.clear();
    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _respawnBlock._charRespawnsUltimate.emplace_back(reader);
    }

    _respawnBlock._charCurrentRespawnNormal = UID16(reader);
    _respawnBlock._charCurrentRespawnElite = UID16(reader);
    _respawnBlock._charCurrentRespawnUltimate = UID16(reader);

    _respawnBlock.ReadBlockEnd(reader);
}

void Character::ReadWaypointBlock(EncodedFileReader* reader)
{
    _waypointBlock.ReadBlockStart(reader);

    _waypointBlock._charWaypointsNormal.clear();
    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _waypointBlock._charWaypointsNormal.emplace_back(reader);
    }

    _waypointBlock._charWaypointsElite.clear();
    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _waypointBlock._charWaypointsElite.emplace_back(reader);
    }

    _waypointBlock._charWaypointsUltimate.clear();
    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _waypointBlock._charWaypointsUltimate.emplace_back(reader);
    }

    _waypointBlock.ReadBlockEnd(reader);
}

void Character::ReadMarkerBlock(EncodedFileReader* reader)
{
    _markerBlock.ReadBlockStart(reader);

    _markerBlock._charMarkersNormal.clear();
    uint32_t numNormalIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numNormalIDs; ++i)
    {
        _markerBlock._charMarkersNormal.emplace_back(reader);
    }

    _markerBlock._charMarkersElite.clear();
    uint32_t numEliteIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numEliteIDs; ++i)
    {
        _markerBlock._charMarkersElite.emplace_back(reader);
    }

    _markerBlock._charMarkersUltimate.clear();
    uint32_t numUltimateIDs = reader->ReadInt32();
    for (uint32_t i = 0; i < numUltimateIDs; ++i)
    {
        _markerBlock._charMarkersUltimate.emplace_back(reader);
    }

    _markerBlock.ReadBlockEnd(reader);
}

void Character::ReadShrineBlock(EncodedFileReader* reader)
{
    _shrineBlock.ReadBlockStart(reader);

    for (uint32_t i = 0; i < 6; ++i)
    {
        _shrineBlock._charShrines[i].clear();
        uint32_t numIDs = reader->ReadInt32();
        for (uint32_t j = 0; j < numIDs; ++j)
        {
            _shrineBlock._charShrines[i].emplace_back(reader);
        }
    }

    _shrineBlock.ReadBlockEnd(reader);
}

void Character::ReadSkillBlock(EncodedFileReader* reader)
{
    _skillBlock.ReadBlockStart(reader);

    _skillBlock._charClassSkills.clear();
    uint32_t numSkills = reader->ReadInt32();
    for (uint32_t i = 0; i < numSkills; ++i)
    {
        _skillBlock._charClassSkills.emplace_back(reader);
    }

    _skillBlock._charMasteriesAllowed = reader->ReadInt32();
    _skillBlock._charSkillReclaimed = reader->ReadInt32();
    _skillBlock._charDevotionReclaimed = reader->ReadInt32();

    _skillBlock._charItemSkills.clear();
    numSkills = reader->ReadInt32();
    for (uint32_t i = 0; i < numSkills; ++i)
    {
        _skillBlock._charItemSkills.emplace_back(reader);
    }

    // Added in version 6
    // Seems to be 0 on most characters
    if (_skillBlock.GetBlockVersion() >= 6)
        _skillBlock._unk1 = reader->ReadInt32();

    _skillBlock.ReadBlockEnd(reader);
}

void Character::ReadNotesBlock(EncodedFileReader* reader)
{
    _notesBlock.ReadBlockStart(reader);

    _notesBlock._charNotes.clear();
    uint32_t numNotes = reader->ReadInt32();
    for (uint32_t i = 0; i < numNotes; ++i)
    {
        _notesBlock._charNotes.push_back(reader->ReadString());
    }

    _notesBlock.ReadBlockEnd(reader);
}

void Character::ReadFactionBlock(EncodedFileReader* reader)
{
    _factionBlock.ReadBlockStart(reader);

    _factionBlock._unk1 = reader->ReadInt32();

    _factionBlock._charFactions.clear();
    uint32_t numFactions = reader->ReadInt32();
    for (uint32_t i = 0; i < numFactions; ++i)
    {
        _factionBlock._charFactions.emplace_back(reader);
        _factionBlock._charFactions[i]._factionID = static_cast<Factions>(i);
    }

    _factionBlock.ReadBlockEnd(reader);
}

void Character::ReadUIBlock(EncodedFileReader* reader)
{
    _UIBlock.ReadBlockStart(reader);

    _UIBlock._unk1 = reader->ReadInt8();
    _UIBlock._unk2 = reader->ReadInt32();
    _UIBlock._unk3 = reader->ReadInt8();

    // Added in version 6, seems to always be 5 of these data structures?
    if (_UIBlock.GetBlockVersion() >= 6)
    {
        _UIBlock._unk4.clear();
        for (uint32_t i = 0; i < 5; ++i)
        {
            CharacterUIBlock::CharacterUIUnkData unknown;
            unknown._unk1 = reader->ReadString();
            unknown._unk2 = reader->ReadString();
            unknown._unk3 = reader->ReadInt8();
            _UIBlock._unk4.push_back(unknown);
        }
    }
    
    _UIBlock._charUISlots.clear();
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
            // Unknown case, hasn't appeared before 1.2.1.0
            case 1:
                reader->ReadInt32();    // Value is "47" on most characters
                reader->ReadInt32();    // Value is "0" on most characters
                break;
            // Not sure what these are; they appear to contain no data other than the slot type
            case 2:
            case 3:
            case 5:     // Seems to be the evade skill?
            case -1:    // Maybe an empty slot?
                break;
            default:
                throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_WARN, "Invalid or unsupported item slot type \"%\"", slot._slotType));
        }
        _UIBlock._charUISlots.push_back(slot);
    }

    // Added in version 7, value is "-1" on most characters
    if (_UIBlock.GetBlockVersion() >= 7)
        _UIBlock._unk6 = reader->ReadInt32();

    // Added in version 6, value is "5" on most characters, probably something to do with the evade/potion buttons or the unknown data above?
    if (_UIBlock.GetBlockVersion() >= 6)
        _UIBlock._unk5 = reader->ReadInt32();

    _UIBlock._charCameraDistance = reader->ReadFloat();

    _UIBlock.ReadBlockEnd(reader);
}

void Character::ReadTutorialBlock(EncodedFileReader* reader)
{
    _tutorialBlock.ReadBlockStart(reader);

    _tutorialBlock._charTutorials.clear();
    uint32_t numTutorials = reader->ReadInt32();
    for (uint32_t i = 0; i < numTutorials; ++i)
    {
        _tutorialBlock._charTutorials.push_back(reader->ReadInt32());
    }

    _tutorialBlock.ReadBlockEnd(reader);
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
}

web::json::value Character::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("HeaderBlock")] = _headerBlock.ToJSON();
    obj[U("InfoBlock")] = _infoBlock.ToJSON();
    obj[U("AttributesBlock")] = _attributesBlock.ToJSON();
    obj[U("InventoryBlock")] = _inventoryBlock.ToJSON();
    obj[U("StashBlock")] = _stashBlock.ToJSON();
    obj[U("RespawnBlock")] = _respawnBlock.ToJSON();
    obj[U("WaypointBlock")] = _waypointBlock.ToJSON();
    obj[U("MarkerBlock")] = _markerBlock.ToJSON();
    obj[U("ShrineBlock")] = _shrineBlock.ToJSON();
    obj[U("SkillBlock")] = _skillBlock.ToJSON();
    obj[U("NotesBlock")] = _notesBlock.ToJSON();
    obj[U("FactionBlock")] = _factionBlock.ToJSON();
    obj[U("UIBlock")] = _UIBlock.ToJSON();
    obj[U("TutorialBlock")] = _tutorialBlock.ToJSON();
    obj[U("StatsBlock")] = _statsBlock.ToJSON();

    return obj;
}

web::json::value Character::CharacterHeaderBlock::ToJSON() const
{
    std::string className = (classNameLookup.count(_charClass) > 0) ? classNameLookup.at(_charClass) : "";

    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("Name")] = web::json::value::string(_charName);
    obj[U("Sex")] = _charSex;
    obj[U("Level")] = _charLevel;
    obj[U("Hardcore")] = _charIsHardcore;
    obj[U("Expansions")] = _charExpansions;
    obj[U("ClassName")] = JSONString(className);
    obj[U("ClassID")] = _charClass;
    obj[U("UID")] = _charUID.ToJSON();

    return obj;
}

web::json::value Character::CharacterInfoBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("IsModCharacter")] = _charIsModded;
    obj[U("IsInGame")] = _charIsInGame;
    obj[U("CurrentDifficulty")] = _charDifficulty;
    obj[U("MaxDifficulty")] = _charMaxDifficulty;
    obj[U("Money")] = _charMoney;
    obj[U("CrucibleDifficulty")] = _charCrucibleDifficulty;
    obj[U("CrucibleTributes")] = _charCrucibleTributes;
    obj[U("CompassState")] = _charCompassState;
    obj[U("LootMode")] = _charLootMode;
    obj[U("SkillWindowHelp")] = _charSkillWindowHelp;
    obj[U("AlternateConfig")] = _charAlternateConfig;
    obj[U("AlternateConfigEnabled")] = _charIsAlternateConfigEnabled;
    obj[U("Texture")] = JSONString(_charTexture);

    web::json::value lootFilter = web::json::value::array();
    for (uint32_t i = 0; i < _charLootFilters.size(); ++i)
    {
        lootFilter[i] = _charLootFilters[i];
    }
    obj[U("LootFilter")] = lootFilter;

    return obj;
}

web::json::value Character::CharacterAttributeBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("Level")] = _charLevel;
    obj[U("Experience")] = _charExperience;
    obj[U("AttributePoints")] = _charAttributePoints;
    obj[U("SkillPoints")] = _charSkillPoints;
    obj[U("DevotionPoints")] = _charDevotionPoints;
    obj[U("TotalDevotionPoints")] = _charTotalDevotionPoints;
    obj[U("Physique")] = _charPhysique;
    obj[U("Cunning")] = _charCunning;
    obj[U("Spirit")] = _charSpirit;
    obj[U("Health")] = _charHealth;
    obj[U("Energy")] = _charEnergy;

    return obj;
}

web::json::value Character::CharacterInventoryBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value inventory = _charInventory.ToJSON();
    inventory[U("FocusedTab")] = _charInventory.GetFocusedTab();
    inventory[U("SelectedTab")] = _charInventory.GetSelectedTab();
    obj[U("Inventory")] = inventory;

    uint32_t i = 0;
    web::json::value equipped = web::json::value::object();
    web::json::value equippedItems = web::json::value::array();
    for (auto pair : _charEquipped.GetItemList())
    {
        uint32_t index = (pair.second & 0xFFFFFFFF);
        web::json::value item = pair.first->ToJSON();
        item[U("Slot")] = index;
        item[U("Attached")] = _charEquipped.GetAttachState(index);
        equippedItems[i++] = item;
    }
    equipped[U("Items")] = equippedItems;
    equipped[U("WeaponSwap")] = _charEquipped.IsUsingSecondaryWeaponSet();
    obj[U("Equipped")] = equipped;

    return obj;
}

web::json::value Character::CharacterStashBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("Stash")] = _charStash.ToJSON();

    return obj;
}

web::json::value Character::CharacterRespawnBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value respawnsNormal = web::json::value::array();
    for (uint32_t i = 0; i < _charRespawnsNormal.size(); ++i)
    {
        respawnsNormal[i] = _charRespawnsNormal[i].ToJSON();
    }
    obj[U("RespawnsNormal")] = respawnsNormal;

    web::json::value respawnsElite = web::json::value::array();;
    for (uint32_t i = 0; i < _charRespawnsElite.size(); ++i)
    {
        respawnsElite[i] = _charRespawnsElite[i].ToJSON();
    }
    obj[U("RespawnsElite")] = respawnsElite;

    web::json::value respawnsUltimate = web::json::value::array();;
    for (uint32_t i = 0; i < _charRespawnsUltimate.size(); ++i)
    {
        respawnsUltimate[i] = _charRespawnsUltimate[i].ToJSON();
    }
    obj[U("RespawnsUltimate")] = respawnsUltimate;

    obj[U("CurrentRespawnNormal")] = _charCurrentRespawnNormal.ToJSON();
    obj[U("CurrentRespawnElite")] = _charCurrentRespawnElite.ToJSON();
    obj[U("CurrentRespawnUltimate")] = _charCurrentRespawnUltimate.ToJSON();

    return obj;
}

web::json::value Character::CharacterWaypointBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value waypointsNormal = web::json::value::array();
    for (uint32_t i = 0; i < _charWaypointsNormal.size(); ++i)
    {
        waypointsNormal[i] = _charWaypointsNormal[i].ToJSON();
    }
    obj[U("WaypointsNormal")] = waypointsNormal;

    web::json::value waypointsElite = web::json::value::array();
    for (uint32_t i = 0; i < _charWaypointsElite.size(); ++i)
    {
        waypointsElite[i] = _charWaypointsElite[i].ToJSON();
    }
    obj[U("WaypointsElite")] = waypointsElite;

    web::json::value waypointsUltimate = web::json::value::array();
    for (uint32_t i = 0; i < _charWaypointsUltimate.size(); ++i)
    {
        waypointsUltimate[i] = _charWaypointsUltimate[i].ToJSON();
    }
    obj[U("WaypointsUltimate")] = waypointsUltimate;

    return obj;
}

web::json::value Character::CharacterMarkerBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value markersNormal = web::json::value::array();
    for (uint32_t i = 0; i < _charMarkersNormal.size(); ++i)
    {
        markersNormal[i] = _charMarkersNormal[i].ToJSON();
    }
    obj[U("MarkersNormal")] = markersNormal;

    web::json::value markersElite = web::json::value::array();
    for (uint32_t i = 0; i < _charMarkersElite.size(); ++i)
    {
        markersElite[i] = _charMarkersElite[i].ToJSON();
    }
    obj[U("MarkersElite")] = markersElite;

    web::json::value markersUltimate = web::json::value::array();
    for (uint32_t i = 0; i < _charMarkersUltimate.size(); ++i)
    {
        markersUltimate[i] = _charMarkersUltimate[i].ToJSON();
    }
    obj[U("MarkersUltimate")] = markersUltimate;

    return obj;
}

web::json::value Character::CharacterShrineBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value shrinesNormalRestored = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[0].size(); ++i)
    {
        shrinesNormalRestored[i] = _charShrines[0][i].ToJSON();
    }
    obj[U("ShrinesNormalRestored")] = shrinesNormalRestored;

    web::json::value shrinesNormalDiscovered = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[1].size(); ++i)
    {
        shrinesNormalDiscovered[i] = _charShrines[1][i].ToJSON();
    }
    obj[U("ShrinesNormalDiscovered")] = shrinesNormalDiscovered;

    web::json::value shrinesEliteRestored = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[2].size(); ++i)
    {
        shrinesEliteRestored[i] = _charShrines[2][i].ToJSON();
    }
    obj[U("ShrinesEliteRestored")] = shrinesEliteRestored;

    web::json::value shrinesEliteDiscovered = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[3].size(); ++i)
    {
        shrinesEliteDiscovered[i] = _charShrines[3][i].ToJSON();
    }
    obj[U("ShrinesEliteDiscovered")] = shrinesEliteDiscovered;

    web::json::value shrinesUltimateRestored = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[4].size(); ++i)
    {
        shrinesUltimateRestored[i] = _charShrines[4][i].ToJSON();
    }
    obj[U("ShrinesUltimateRestored")] = shrinesUltimateRestored;

    web::json::value shrinesUltimateDiscovered = web::json::value::array();
    for (uint32_t i = 0; i < _charShrines[5].size(); ++i)
    {
        shrinesUltimateDiscovered[i] = _charShrines[5][i].ToJSON();
    }
    obj[U("ShrinesUltimateDiscovered")] = shrinesUltimateDiscovered;

    return obj;
}

web::json::value Character::CharacterSkillBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("MasteriesAllowed")] = _charMasteriesAllowed;
    obj[U("SkillPointsReclaimed")] = _charSkillReclaimed;
    obj[U("DevotionPointsReclaimed")] = _charDevotionReclaimed;

    web::json::value classSkills = web::json::value::array();
    for (uint32_t i = 0; i < _charClassSkills.size(); ++i)
    {
        classSkills[i] = _charClassSkills[i].ToJSON();
    }
    obj[U("ClassSkills")] = classSkills;

    web::json::value itemSkills = web::json::value::array();
    for (uint32_t i = 0; i < _charItemSkills.size(); ++i)
    {
        itemSkills[i] = _charItemSkills[i].ToJSON();
    }
    obj[U("ItemSkills")] = itemSkills;

    return obj;
}

web::json::value Character::CharacterNotesBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value notes = web::json::value::array();
    for (uint32_t i = 0; i < _charNotes.size(); ++i)
    {
        notes[i] = JSONString(_charNotes[i]);
    }
    obj[U("Notes")] = notes;

    return obj;
}

web::json::value Character::CharacterFactionBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("Unknown1")] = _unk1;

    web::json::value factions = web::json::value::array();
    for (uint32_t i = 0; i < _charFactions.size(); ++i)
    {
        factions[i] = _charFactions[i].ToJSON();
    }
    obj[U("Factions")] = factions;

    return obj;
}

web::json::value Character::CharacterUIBlock::CharacterUIUnkData::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("UnknownData1")] = JSONString(_unk1);
    obj[U("UnknownData2")] = JSONString(_unk2);
    obj[U("UnknownData3")] = _unk3;

    return obj;
}

web::json::value Character::CharacterUIBlock::CharacterUISlot::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("SlotType")] = _slotType;
    obj[U("SkillName")] = JSONString(_slotSkillName);
    obj[U("ItemName")] = JSONString(_slotItemName);
    obj[U("IsItemSkill")] = _slotIsItemSkill;
    obj[U("EquipSlot")] = _slotEquip;
    obj[U("BitmapUp")] = JSONString(_slotBitmapUp);
    obj[U("BitmapDown")] = JSONString(_slotBitmapDown);
    obj[U("Label")] = web::json::value::string(_slotLabel);

    return obj;
}

web::json::value Character::CharacterUIBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("Unknown1")] = _unk1;
    obj[U("Unknown2")] = _unk2;
    obj[U("Unknown3")] = _unk3;
    obj[U("CameraDistance")] = _charCameraDistance;

    web::json::value unknown = web::json::value::array();
    for (uint32_t i = 0; i < _unk4.size(); ++i)
    {
        unknown[i] = _unk4[i].ToJSON();
    }
    obj[U("Unknown")] = unknown;

    web::json::value UISlots = web::json::value::array();
    for (uint32_t i = 0; i < _charUISlots.size(); ++i)
    {
        UISlots[i] = _charUISlots[i].ToJSON();
    }
    obj[U("UISlots")] = UISlots;

    return obj;
}

web::json::value Character::CharacterTutorialBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    web::json::value tutorials = web::json::value::array();
    for (uint32_t i = 0; i < _charTutorials.size(); ++i)
    {
        tutorials[i] = _charTutorials[i];
    }
    obj[U("Tutorials")] = tutorials;

    return obj;
}

web::json::value Character::CharacterStatsBlock::CharacterPerDifficultyStats::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("Difficulty")] = _difficulty;
    obj[U("GreatestEnemyKilled")] = JSONString(_greatestEnemyKilled);
    obj[U("GreatestEnemyLevel")] = _greatestEnemyLevel;
    obj[U("GreatestEnemyHealth")] = _greatestEnemyHealth;
    obj[U("LastAttacked")] = JSONString(_lastAttacked);
    obj[U("LastAttackedBy")] = JSONString(_lastAttackedBy);
    obj[U("NemesisKills")] = _nemesisKills;

    return obj;
}

web::json::value Character::CharacterStatsBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    obj[U("PlayedTime")] = _charPlayTime;
    obj[U("Deaths")] = _charDeaths;
    obj[U("Kills")] = _charKills;
    obj[U("ExpFromKills")] = _charExpFromKills;
    obj[U("HealthPotsUsed")] = _charHealthPotsUsed;
    obj[U("ManaPotsUsed")] = _charManaPotsUsed;
    obj[U("MaxLevel")] = _charMaxLevel;
    obj[U("HitsReceived")] = _charHitsReceived;
    obj[U("HitsInflicted")] = _charHitsInflicted;
    obj[U("CritsReceived")] = _charCritsReceived;
    obj[U("CritsInflicted")] = _charCritsInflicted;
    obj[U("LargestHitReceived")] = _charGreatestDamageReceived;
    obj[U("LargestHitInflicted")] = _charGreatestDamageInflicted;
    obj[U("ChampionKills")] = _charChampionKills;
    obj[U("HeroKills")] = _charHeroKills;
    obj[U("ItemsCrafted")] = _charItemsCrafted;
    obj[U("RelicsCrafted")] = _charRelicsCrafted;
    obj[U("TranscendentRelicsCrafted")] = _charTranscendentRelicsCrafted;
    obj[U("MythicalRelicsCrafted")] = _charMythicalRelicsCrafted;
    obj[U("ShrinesRestored")] = _charShrinesRestored;
    obj[U("OneShotChestsOpened")] = _charOneShotChestsOpened;
    obj[U("LoreNotesCollected")] = _charLoreNotesCollected;
    obj[U("Unknown1")] = _unk1;
    obj[U("Unknown2")] = _unk2;
    obj[U("LastAttackedDA")] = _charLastAttackedDA;
    obj[U("LastAttackedByOA")] = _charLastAttackedByOA;

    web::json::value perDifficultyStats = web::json::value::array();
    for (uint32_t i = 0; i < 3; ++i)
    {
        perDifficultyStats[i] = _charDifficultyStats[i].ToJSON();
    }
    obj[U("PerDifficultyStats")] = perDifficultyStats;

    obj[U("CrucibleGreatestWave")] = _charCrucibleGreatestWave;
    obj[U("CrucibleGreatestScore")] = _charCrucibleGreatestScore;
    obj[U("CrucibleDefensesBuilt")] = _charCrucibleDefensesBuilt;
    obj[U("CrucibleBuffsUsed")] = _charCrucibleBuffsUsed;

    uint32_t j = 0;
    web::json::value SRShrinesUsed = web::json::value::object();
    for (std::pair<std::string, uint32_t> pair : _charSRShrinesUsed)
    {
        SRShrinesUsed[utility::conversions::to_utf16string(pair.first)] = pair.second;
    }
    obj[U("SRShrinesUsed")] = SRShrinesUsed;
    obj[U("SRSoulsCollected")] = _charSRSoulsCollected;
    obj[U("SRFlag")] = _charSRFlag;
    obj[U("MeritUsed")] = _charMeritUsed;

    return obj;
}