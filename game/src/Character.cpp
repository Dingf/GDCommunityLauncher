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

std::string GetCharacterClassName(CharacterClass charClass)
{
    if (classNameLookup.count(charClass) > 0)
    {
        return classNameLookup.at(charClass);
    }
    else
    {
        return {};
    }
}

bool Character::ReadFromFile(const std::filesystem::path& path, bool headerOnly)
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
            if (!headerOnly)
            {
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
            }
            return true;
        }
        catch (std::runtime_error& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character file \"%\": %", path.string().c_str(), ex.what());
            return false;
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "The path \"%\" is not recognized as a file.", path.string());
    }
    return false;
}

bool Character::ReadFromBuffer(const uint8_t* data, size_t size, bool headerOnly)
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
        if (!headerOnly)
        {
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
        }
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

    std::string className = GetCharacterClassName(_headerBlock._charClass);
    if (className.empty())
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
    _statsBlock._charCritsReceived = reader->ReadInt32();
    _statsBlock._charCritsInflicted = reader->ReadInt32();
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

void to_json(json& j, const Character& data)
{
    j = json
    {
        { "HeaderBlock",     data._headerBlock },
        { "InfoBlock",       data._infoBlock },
        { "AttributesBlock", data._attributesBlock },
        { "InventoryBlock",  data._inventoryBlock },
        { "StashBlock",      data._stashBlock },
        { "RespawnBlock",    data._respawnBlock },
        { "WaypointBlock",   data._waypointBlock },
        { "MarkerBlock",     data._markerBlock },
        { "ShrineBlock",     data._shrineBlock },
        { "SkillBlock",      data._skillBlock },
        { "NotesBlock",      data._notesBlock },
        { "FactionBlock",    data._factionBlock },
        { "UIBlock",         data._UIBlock },
        { "TutorialBlock",   data._tutorialBlock },
        { "StatsBlock",      data._statsBlock },
    };
}

void from_json(const json& j, Character& data)
{
    j.at("HeaderBlock")    .get_to(data._headerBlock);
    j.at("InfoBlock")      .get_to(data._infoBlock);
    j.at("AttributesBlock").get_to(data._attributesBlock);
    j.at("InventoryBlock") .get_to(data._inventoryBlock);
    j.at("StashBlock")     .get_to(data._stashBlock);
    j.at("RespawnBlock")   .get_to(data._respawnBlock);
    j.at("WaypointBlock")  .get_to(data._waypointBlock);
    j.at("MarkerBlock")    .get_to(data._markerBlock);
    j.at("ShrineBlock")    .get_to(data._shrineBlock);
    j.at("SkillBlock")     .get_to(data._skillBlock);
    j.at("NotesBlock")     .get_to(data._notesBlock);
    j.at("FactionBlock")   .get_to(data._factionBlock);
    j.at("UIBlock")        .get_to(data._UIBlock);
    j.at("TutorialBlock")  .get_to(data._tutorialBlock);
    j.at("StatsBlock")     .get_to(data._statsBlock);
}

void to_json(json& j, const Character::CharacterHeaderBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Name",         data._charName },
        { "Sex",          data._charSex },
        { "Level",        data._charLevel },
        { "Hardcore",     data._charIsHardcore },
        { "Expansions",   data._charExpansions },
        { "ClassName",    GetCharacterClassName(data._charClass) },
        { "ClassID",      data._charClass },
        { "UID",          data._charUID },
    };
}

void from_json(const json& j, Character::CharacterHeaderBlock& data)
{
    j.at("Name")      .get_to(data._charName);
    j.at("Sex")       .get_to(data._charSex);
    j.at("Level")     .get_to(data._charLevel);
    j.at("Hardcore")  .get_to(data._charIsHardcore);
    j.at("Expansions").get_to(data._charExpansions);
    j.at("ClassID")   .get_to(data._charClass);
    j.at("UID")       .get_to(data._charUID);
}

void to_json(json& j, const Character::CharacterInfoBlock& data)
{
    j = json
    {
        { "BlockID",                data.GetBlockID() },
        { "BlockVersion",           data.GetBlockVersion() },
        { "IsModCharacter",         data._charIsModded },
        { "IsInGame",               data._charIsInGame },
        { "CurrentDifficulty",      data._charDifficulty },
        { "MaxDifficulty",          data._charMaxDifficulty },
        { "Money",                  data._charMoney },
        { "CrucibleDifficulty",     data._charCrucibleDifficulty },
        { "CrucibleTributes",       data._charCrucibleTributes },
        { "CompassState",           data._charCompassState },
        { "LootMode",               data._charLootMode },
        { "SkillWindowHelp",        data._charSkillWindowHelp },
        { "AlternateConfig",        data._charAlternateConfig },
        { "AlternateConfigEnabled", data._charIsAlternateConfigEnabled },
        { "Texture",                data._charTexture },
        { "LootFilter",             data._charLootFilters },
    };
}

void from_json(const json& j, Character::CharacterInfoBlock& data)
{
    j.at("IsModCharacter")        .get_to(data._charIsModded);
    j.at("IsInGame")              .get_to(data._charIsInGame);
    j.at("CurrentDifficulty")     .get_to(data._charDifficulty);
    j.at("MaxDifficulty")         .get_to(data._charMaxDifficulty);
    j.at("Money")                 .get_to(data._charMoney);
    j.at("CrucibleDifficulty")    .get_to(data._charCrucibleDifficulty);
    j.at("CrucibleTributes")      .get_to(data._charCrucibleTributes);
    j.at("CompassState")          .get_to(data._charCompassState);
    j.at("LootMode")              .get_to(data._charLootMode);
    j.at("SkillWindowHelp")       .get_to(data._charSkillWindowHelp);
    j.at("AlternateConfig")       .get_to(data._charAlternateConfig);
    j.at("AlternateConfigEnabled").get_to(data._charIsAlternateConfigEnabled);
    j.at("Texture")               .get_to(data._charTexture);
    j.at("LootFilter")            .get_to(data._charLootFilters);
}

void to_json(json& j, const Character::CharacterAttributesBlock& data)
{
    j = json
    {
        { "BlockID",             data.GetBlockID() },
        { "BlockVersion",        data.GetBlockVersion() },
        { "Level",               data._charLevel },
        { "Experience",          data._charExperience },
        { "AttributePoints",     data._charAttributePoints },
        { "SkillPoints",         data._charSkillPoints },
        { "DevotionPoints",      data._charDevotionPoints },
        { "TotalDevotionPoints", data._charTotalDevotionPoints },
        { "Physique",            data._charPhysique },
        { "Cunning",             data._charCunning },
        { "Spirit",              data._charSpirit },
        { "Health",              data._charHealth },
        { "Energy",              data._charEnergy },
    };
}

void from_json(const json& j, Character::CharacterAttributesBlock& data)
{
    j.at("Level")              .get_to(data._charLevel);
    j.at("Experience")         .get_to(data._charExperience);
    j.at("AttributePoints")    .get_to(data._charAttributePoints);
    j.at("SkillPoints")        .get_to(data._charSkillPoints);
    j.at("DevotionPoints")     .get_to(data._charDevotionPoints);
    j.at("TotalDevotionPoints").get_to(data._charTotalDevotionPoints);
    j.at("Physique")           .get_to(data._charPhysique);
    j.at("Cunning")            .get_to(data._charCunning);
    j.at("Spirit")             .get_to(data._charSpirit);
    j.at("Health")             .get_to(data._charHealth);
    j.at("Energy")             .get_to(data._charEnergy);
}

void to_json(json& j, const Character::CharacterInventoryBlock& data)
{
    json equippedItems;
    for (auto pair : data._charEquipped.GetItemList())
    {
        uint32_t slot = (pair.second & 0xFFFFFFFF);
        json item = *pair.first;
        item["Slot"] = slot;
        item["Attached"] = data._charEquipped.GetAttachState(slot);
        equippedItems.push_back(item);
    }

    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "FocusedTab",   data._charInventory.GetFocusedTab() },
        { "SelectedTab",  data._charInventory.GetSelectedTab() },
        { "Inventory",    data._charInventory },
        { "Equipped", {
            { "Items",      equippedItems },
            { "WeaponSwap", data._charEquipped.IsUsingSecondaryWeaponSet() }
        }},
    };
}

void from_json(const json& j, Character::CharacterInventoryBlock& data)
{
    j.at("Inventory").get_to(data._charInventory);
    data._charInventory.SetFocusedTab(j.at("FocusedTab").get<uint32_t>());
    data._charInventory.SetSelectedTab(j.at("SelectedTab").get<uint32_t>());
    
    json equippedItems = j.at("Equipped").at("Items");
    for (auto it = equippedItems.begin(); it != equippedItems.end(); ++it)
    {
        Item item = it->get<Item>();
        uint32_t slot = it->at("Slot");

        data._charEquipped.AddItem(item, 0, slot);
        data._charEquipped.SetAttachState(slot, it->at("Attached"));
    }
    data._charEquipped.SetActiveWeaponSet(j.at("Equipped").at("WeaponSwap").get<bool>());
}

void to_json(json& j, const Character::CharacterStashBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Stash",        data._charStash },
    };
}

void from_json(const json& j, Character::CharacterStashBlock& data)
{
    j.at("Stash").get_to(data._charStash);
}

void to_json(json& j, const Character::CharacterRespawnBlock& data)
{
    j = json
    {
        { "BlockID",                data.GetBlockID() },
        { "BlockVersion",           data.GetBlockVersion() },
        { "RespawnsNormal",         data._charRespawnsNormal },
        { "RespawnsElite",          data._charRespawnsElite },
        { "RespawnsUltimate",       data._charRespawnsUltimate },
        { "CurrentRespawnNormal",   data._charCurrentRespawnNormal },
        { "CurrentRespawnElite",    data._charCurrentRespawnElite },
        { "CurrentRespawnUltimate", data._charCurrentRespawnUltimate },
    };
}

void from_json(const json& j, Character::CharacterRespawnBlock& data)
{
    j.at("RespawnsNormal")        .get_to(data._charRespawnsNormal);
    j.at("RespawnsElite")         .get_to(data._charRespawnsElite);
    j.at("RespawnsUltimate")      .get_to(data._charRespawnsUltimate);
    j.at("CurrentRespawnNormal")  .get_to(data._charCurrentRespawnNormal);
    j.at("CurrentRespawnElite")   .get_to(data._charCurrentRespawnElite);
    j.at("CurrentRespawnUltimate").get_to(data._charCurrentRespawnUltimate);
}

void to_json(json& j, const Character::CharacterWaypointBlock& data)
{
    j = json
    {
        { "BlockID",           data.GetBlockID() },
        { "BlockVersion",      data.GetBlockVersion() },
        { "WaypointsNormal",   data._charWaypointsNormal },
        { "WaypointsElite",    data._charWaypointsElite },
        { "WaypointsUltimate", data._charWaypointsUltimate },
    };
}

void from_json(const json& j, Character::CharacterWaypointBlock& data)
{
    j.at("WaypointsNormal")  .get_to(data._charWaypointsNormal);
    j.at("WaypointsElite")   .get_to(data._charWaypointsElite);
    j.at("WaypointsUltimate").get_to(data._charWaypointsUltimate);
}

void to_json(json& j, const Character::CharacterMarkerBlock& data)
{
    j = json
    {
        { "BlockID",         data.GetBlockID() },
        { "BlockVersion",    data.GetBlockVersion() },
        { "MarkersNormal",   data._charMarkersNormal },
        { "MarkersElite",    data._charMarkersElite },
        { "MarkersUltimate", data._charMarkersUltimate },
    };
}

void from_json(const json& j, Character::CharacterMarkerBlock& data)
{
    j.at("MarkersNormal")  .get_to(data._charMarkersNormal);
    j.at("MarkersElite")   .get_to(data._charMarkersElite);
    j.at("MarkersUltimate").get_to(data._charMarkersUltimate);
}

void to_json(json& j, const Character::CharacterShrineBlock& data)
{
    j = json
    {
        { "BlockID",                   data.GetBlockID() },
        { "BlockVersion",              data.GetBlockVersion() },
        { "ShrinesNormalRestored",     data._charShrines[0] },
        { "ShrinesNormalDiscovered",   data._charShrines[1] },
        { "ShrinesEliteRestored",      data._charShrines[2] },
        { "ShrinesEliteDiscovered",    data._charShrines[3] },
        { "ShrinesUltimateRestored",   data._charShrines[4] },
        { "ShrinesUltimateDiscovered", data._charShrines[5] },
    };
}

void from_json(const json& j, Character::CharacterShrineBlock& data)
{
    j.at("ShrinesNormalRestored")    .get_to(data._charShrines[0]);
    j.at("ShrinesNormalDiscovered")  .get_to(data._charShrines[1]);
    j.at("ShrinesEliteRestored")     .get_to(data._charShrines[2]);
    j.at("ShrinesEliteDiscovered")   .get_to(data._charShrines[3]);
    j.at("ShrinesUltimateRestored")  .get_to(data._charShrines[4]);
    j.at("ShrinesUltimateDiscovered").get_to(data._charShrines[5]);
}

void to_json(json& j, const Character::CharacterSkillBlock& data)
{
    j = json
    {
        { "BlockID",                 data.GetBlockID() },
        { "BlockVersion",            data.GetBlockVersion() },
        { "MasteriesAllowed",        data._charMasteriesAllowed },
        { "SkillPointsReclaimed",    data._charSkillReclaimed },
        { "DevotionPointsReclaimed", data._charDevotionReclaimed },
        { "ClassSkills",             data._charClassSkills },
        { "ItemSkills",              data._charItemSkills },
    };
}

void from_json(const json& j, Character::CharacterSkillBlock& data)
{
    j.at("MasteriesAllowed")       .get_to(data._charMasteriesAllowed);
    j.at("SkillPointsReclaimed")   .get_to(data._charSkillReclaimed);
    j.at("DevotionPointsReclaimed").get_to(data._charDevotionReclaimed);
    j.at("ClassSkills")            .get_to(data._charClassSkills);
    j.at("ItemSkills")             .get_to(data._charItemSkills);
}

void to_json(json& j, const Character::CharacterNotesBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Notes",        data._charNotes },
    };
}

void from_json(const json& j, Character::CharacterNotesBlock& data)
{
    j.at("Notes").get_to(data._charNotes);
}

void to_json(json& j, const Character::CharacterFactionBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Unknown1",     data._unk1 },
        { "Factions",     data._charFactions },
    };
}

void from_json(const json& j, Character::CharacterFactionBlock& data)
{
    j.at("Unknown1").get_to(data._unk1);
    j.at("Factions").get_to(data._charFactions);
}

void to_json(json& j, const Character::CharacterUIBlock& data)
{
    j = json
    {
        { "BlockID",        data.GetBlockID() },
        { "BlockVersion",   data.GetBlockVersion() },
        { "Unknown1",       data._unk1 },
        { "Unknown2",       data._unk2 },
        { "Unknown3",       data._unk3 },
        { "CameraDistance", data._charCameraDistance },
        { "Unknown4",       data._unk4 },
        { "UISlots",        data._charUISlots },
    };
}

void from_json(const json& j, Character::CharacterUIBlock& data)
{
    j.at("Unknown1")      .get_to(data._unk1);
    j.at("Unknown2")      .get_to(data._unk2);
    j.at("Unknown3")      .get_to(data._unk3);
    j.at("CameraDistance").get_to(data._charCameraDistance);
    j.at("Unknown4")      .get_to(data._unk4);
    j.at("UISlots")       .get_to(data._charUISlots);
}

void to_json(json& j, const Character::CharacterUIBlock::CharacterUIUnkData& data)
{
    j = 
    {
        { "Unknown1", data._unk1 },
        { "Unknown2", data._unk2 },
        { "Unknown3", data._unk3 },
    };
}

void from_json(const json& j, Character::CharacterUIBlock::CharacterUIUnkData& data)
{
    j.at("Unknown1").get_to(data._unk1);
    j.at("Unknown2").get_to(data._unk2);
    j.at("Unknown3").get_to(data._unk3);
}

void to_json(json& j, const Character::CharacterUIBlock::CharacterUISlot& data)
{
    j = 
    {
        { "SlotType",    data._slotType },
        { "SkillName",   data._slotSkillName },
        { "ItemName",    data._slotItemName },
        { "IsItemSkill", data._slotIsItemSkill },
        { "EquipSlot",   data._slotEquip },
        { "BitmapUp",    data._slotBitmapUp },
        { "BitmapDown",  data._slotBitmapDown },
        { "Label",       data._slotLabel },
    };
}

void from_json(const json& j, Character::CharacterUIBlock::CharacterUISlot& data)
{
    j.at("SlotType")   .get_to(data._slotType);
    j.at("SkillName")  .get_to(data._slotSkillName);
    j.at("ItemName")   .get_to(data._slotItemName);
    j.at("IsItemSkill").get_to(data._slotIsItemSkill);
    j.at("EquipSlot")  .get_to(data._slotEquip);
    j.at("BitmapUp")   .get_to(data._slotBitmapUp);
    j.at("BitmapDown") .get_to(data._slotBitmapDown);
    j.at("Label")      .get_to(data._slotLabel);
}

void to_json(json& j, const Character::CharacterTutorialBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Tutorials",    data._charTutorials },
    };
}

void from_json(const json& j, Character::CharacterTutorialBlock& data)
{
    j.at("Tutorials").get_to(data._charTutorials);
}

void to_json(json& j, const Character::CharacterStatsBlock& data)
{
    j = json
    {
        { "BlockID",                   data.GetBlockID() },
        { "BlockVersion",              data.GetBlockVersion() },
        { "PlayedTime",                data._charPlayTime },
        { "Deaths",                    data._charDeaths },
        { "Kills",                     data._charKills },
        { "ExpFromKills",              data._charExpFromKills },
        { "HealthPotsUsed",            data._charHealthPotsUsed },
        { "ManaPotsUsed",              data._charManaPotsUsed },
        { "MaxLevel",                  data._charMaxLevel },
        { "HitsReceived",              data._charHitsReceived },
        { "HitsInflicted",             data._charHitsInflicted },
        { "CritsReceived",             data._charCritsReceived },
        { "CritsInflicted",            data._charCritsInflicted },
        { "LargestHitReceived",        data._charGreatestDamageReceived },
        { "LargestHitInflicted",       data._charGreatestDamageInflicted },
        { "ChampionKills",             data._charChampionKills },
        { "HeroKills",                 data._charHeroKills },
        { "ItemsCrafted",              data._charItemsCrafted },
        { "RelicsCrafted",             data._charRelicsCrafted },
        { "TranscendentRelicsCrafted", data._charTranscendentRelicsCrafted },
        { "MythicalRelicsCrafted",     data._charMythicalRelicsCrafted },
        { "ShrinesRestored",           data._charShrinesRestored },
        { "OneShotChestsOpened",       data._charOneShotChestsOpened },
        { "LoreNotesCollected",        data._charLoreNotesCollected },
        { "Unknown1",                  data._unk1 },
        { "Unknown2",                  data._unk2 },
        { "LastAttackedDA",            data._charLastAttackedDA },
        { "LastAttackedByOA",          data._charLastAttackedByOA },
        { "PerDifficultyStats",        data._charDifficultyStats },
        { "CrucibleGreatestWave",      data._charCrucibleGreatestWave },
        { "CrucibleGreatestScore",     data._charCrucibleGreatestScore },
        { "CrucibleDefensesBuilt",     data._charCrucibleDefensesBuilt },
        { "CrucibleBuffsUsed",         data._charCrucibleBuffsUsed },
        { "SRShrinesUsed",             data._charSRShrinesUsed },
        { "SRSoulsCollected",          data._charSRSoulsCollected },
        { "SRFlag",                    data._charSRFlag },
        { "MeritUsed",                 data._charMeritUsed },
    };
}

void from_json(const json& j, Character::CharacterStatsBlock& data)
{
    j.at("PlayedTime")               .get_to(data._charPlayTime);
    j.at("Deaths")                   .get_to(data._charDeaths);
    j.at("Kills")                    .get_to(data._charKills);
    j.at("ExpFromKills")             .get_to(data._charExpFromKills);
    j.at("HealthPotsUsed")           .get_to(data._charHealthPotsUsed);
    j.at("ManaPotsUsed")             .get_to(data._charManaPotsUsed);
    j.at("MaxLevel")                 .get_to(data._charMaxLevel);
    j.at("HitsReceived")             .get_to(data._charHitsReceived);
    j.at("HitsInflicted")            .get_to(data._charHitsInflicted);
    j.at("CritsReceived")            .get_to(data._charCritsReceived);
    j.at("CritsInflicted")           .get_to(data._charCritsInflicted);
    j.at("LargestHitReceived")       .get_to(data._charGreatestDamageReceived);
    j.at("LargestHitInflicted")      .get_to(data._charGreatestDamageInflicted);
    j.at("ChampionKills")            .get_to(data._charChampionKills);
    j.at("HeroKills")                .get_to(data._charHeroKills);
    j.at("ItemsCrafted")             .get_to(data._charItemsCrafted);
    j.at("RelicsCrafted")            .get_to(data._charRelicsCrafted);
    j.at("TranscendentRelicsCrafted").get_to(data._charTranscendentRelicsCrafted);
    j.at("MythicalRelicsCrafted")    .get_to(data._charMythicalRelicsCrafted);
    j.at("ShrinesRestored")          .get_to(data._charShrinesRestored);
    j.at("OneShotChestsOpened")      .get_to(data._charOneShotChestsOpened);
    j.at("LoreNotesCollected")       .get_to(data._charLoreNotesCollected);
    j.at("Unknown1")                 .get_to(data._unk1);
    j.at("Unknown2")                 .get_to(data._unk2);
    j.at("LastAttackedDA")           .get_to(data._charLastAttackedDA);
    j.at("LastAttackedByOA")         .get_to(data._charLastAttackedByOA);
    j.at("PerDifficultyStats")       .get_to(data._charDifficultyStats);
    j.at("CrucibleGreatestWave")     .get_to(data._charCrucibleGreatestWave);
    j.at("CrucibleGreatestScore")    .get_to(data._charCrucibleGreatestScore);
    j.at("CrucibleDefensesBuilt")    .get_to(data._charCrucibleDefensesBuilt);
    j.at("CrucibleBuffsUsed")        .get_to(data._charCrucibleBuffsUsed);
    j.at("SRShrinesUsed")            .get_to(data._charSRShrinesUsed);
    j.at("SRSoulsCollected")         .get_to(data._charSRSoulsCollected);
    j.at("SRFlag")                   .get_to(data._charSRFlag);
    j.at("MeritUsed")                .get_to(data._charMeritUsed);
}

void to_json(json& j, const Character::CharacterStatsBlock::CharacterPerDifficultyStats& data)
{
    j = json
    {
        { "Difficulty",          data._difficulty },
        { "GreatestEnemyKilled", data._greatestEnemyKilled },
        { "GreatestEnemyLevel",  data._greatestEnemyLevel },
        { "GreatestEnemyHealth", data._greatestEnemyHealth },
        { "LastAttacked",        data._lastAttacked },
        { "LastAttackedBy",      data._lastAttackedBy },
        { "NemesisKills",        data._nemesisKills },
    };
}

void from_json(const json& j, Character::CharacterStatsBlock::CharacterPerDifficultyStats& data)
{
    j.at("Difficulty")         .get_to(data._difficulty);
    j.at("GreatestEnemyKilled").get_to(data._greatestEnemyKilled);
    j.at("GreatestEnemyLevel") .get_to(data._greatestEnemyLevel);
    j.at("GreatestEnemyHealth").get_to(data._greatestEnemyHealth);
    j.at("LastAttacked")       .get_to(data._lastAttacked);
    j.at("LastAttackedBy")     .get_to(data._lastAttackedBy);
    j.at("NemesisKills")       .get_to(data._nemesisKills);
}