#include <fstream>
#include <filesystem>
#include "TEXImage.h"
#include "ItemDBR.h"
#include "Log.h"

// List of different variable names that could point to the item bitmap
const std::vector<std::string> ItemDBR::_bitmapVariables =
{
    "bitmap",
    "artifactBitmap",
    "relicBitmap",
    "noteBitmap",
    "artifactFormulaBitmapName",
};

// List of directories to search for item DBRs
const std::vector<std::string> ItemDBR::_searchPaths =
{
    "records/items",
    "records/storyelements",      // For some reason Lokarr's set is stored here instead of in records/items
    "records/storyelementsgdx2",
    "records/endlessdungeon/items",
    "records/endlessdungeon/scriptentities",
    "records/creatures/npcs/npcgear",
    "grimleague/items",
    "grimleague/infinitydungeons"
};

const std::map<std::string, ItemTypePair> itemTypes =
{
    { "ArmorProtective_Head",      { ITEM_TYPE_HEAD,      WEAPON_TYPE_DEFAULT } },
    { "ArmorJewelry_Amulet",       { ITEM_TYPE_AMULET,    WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Chest",     { ITEM_TYPE_CHEST,     WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Legs",      { ITEM_TYPE_LEGS,      WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Feet",      { ITEM_TYPE_FEET,      WEAPON_TYPE_DEFAULT } },
    { "ArmorJewelry_Ring",         { ITEM_TYPE_RING,      WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Hands",     { ITEM_TYPE_HANDS,     WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Waist",     { ITEM_TYPE_BELT,      WEAPON_TYPE_DEFAULT } },
    { "ArmorProtective_Shoulders", { ITEM_TYPE_SHOULDERS, WEAPON_TYPE_DEFAULT } },
    { "ArmorJewelry_Medal",        { ITEM_TYPE_MEDAL,     WEAPON_TYPE_DEFAULT } },
    { "WeaponMelee_Sword",         { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SWORD_1H } },
    { "WeaponMelee_Sword2h",       { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SWORD_2H } },
    { "WeaponMelee_Axe",           { ITEM_TYPE_WEAPON,    WEAPON_TYPE_AXE_1H } },
    { "WeaponMelee_Axe2h",         { ITEM_TYPE_WEAPON,    WEAPON_TYPE_AXE_2H } },
    { "WeaponMelee_Mace",          { ITEM_TYPE_WEAPON,    WEAPON_TYPE_MACE_1H } },
    { "WeaponMelee_Mace2h",        { ITEM_TYPE_WEAPON,    WEAPON_TYPE_MACE_2H } },
    { "WeaponMelee_Dagger",        { ITEM_TYPE_WEAPON,    WEAPON_TYPE_DAGGER } },
    { "WeaponMelee_Scepter",       { ITEM_TYPE_WEAPON,    WEAPON_TYPE_SCEPTER } },
    { "WeaponHunting_Ranged1h",    { ITEM_TYPE_WEAPON,    WEAPON_TYPE_RANGED_1H } },
    { "WeaponHunting_Ranged2h",    { ITEM_TYPE_WEAPON,    WEAPON_TYPE_RANGED_2H } },
    { "WeaponArmor_Shield",        { ITEM_TYPE_OFFHAND,   WEAPON_TYPE_SHIELD } },
    { "WeaponArmor_Offhand",       { ITEM_TYPE_OFFHAND,   WEAPON_TYPE_CASTER_OH } },
    { "ItemArtifact",              { ITEM_TYPE_RELIC,     WEAPON_TYPE_DEFAULT } }
};

void ItemDBR::BuildItemDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath)
{
    if (!std::filesystem::is_directory(dataPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", dataPath.string().c_str()));

    if (!std::filesystem::is_directory(outPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", outPath.string().c_str()));

    std::vector<std::unique_ptr<ItemDBR>> itemDBRs;
    for (uint32_t i = 0; i < _searchPaths.size(); ++i)
    {
        std::filesystem::path searchPath = dataPath / _searchPaths[i];
        for (auto& entry : std::filesystem::recursive_directory_iterator(searchPath))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".dbr"))
            {
                std::unique_ptr<ItemDBR> itemDBR = std::make_unique<ItemDBR>(entry.path());
                itemDBRs.push_back(std::move(itemDBR));
            }
        }
    }

    if (itemDBRs.empty())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find any item DBRs in %", dataPath.string().c_str()));

    std::filesystem::path outFilePath = outPath / "ItemDatabase.txt";

    std::ofstream out(outFilePath, std::ofstream::out);
    if (!out.is_open())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open file % for writing", outFilePath.string().c_str()));

    //TODO: Also extract the item bitmaps and save them for later use?
    for (uint32_t i = 0; i < itemDBRs.size(); ++i)
    {
        ItemTypePair type = itemDBRs[i]->GetType();
        uint32_t width = itemDBRs[i]->GetWidth();
        uint32_t height = itemDBRs[i]->GetHeight();
        if ((width > 0) && (height > 0))
        {
            std::string itemName = itemDBRs[i]->GetRecordPath().string();
            std::replace(itemName.begin(), itemName.end(), '\\', '/');
            out << "\"" << itemName << "\" " << type._itemType << " " << type._weaponType << " " << width / 32 << " " << height / 32 << "\n";
        }
    }

    out.close();
}

ItemDBR::ItemDBR(const std::filesystem::path& path, bool loadBitmap) : _width(0), _height(0)
{
    if (!DBRecord::Load(path))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid item DBR file", path.string().c_str()));

    const Value* bitmapPath = nullptr;
    for (uint32_t i = 0; i < _bitmapVariables.size(); ++i)
    {
        bitmapPath = GetVariable(_bitmapVariables[i]);
        if (bitmapPath)
            break;
    }

    const Value* itemClass = GetVariable("Class");
    if (itemClass)
    {
        std::string itemClassName = itemClass->ToString();
        if (itemTypes.count(itemClassName) > 0)
        {
            _type = itemTypes.at(itemClassName);
        }
        else
        {
            _type._itemType = ITEM_TYPE_DEFAULT;
            _type._weaponType = WEAPON_TYPE_DEFAULT;
        }
    }

    if ((bitmapPath) && (loadBitmap))
    {
        std::string textureString = bitmapPath->ToString();
        if (textureString.front() == '/')
            textureString = std::string(textureString.begin() + 1, textureString.end());

        std::filesystem::path texturePath = GetRootPath() / textureString;
        if (std::filesystem::is_regular_file(texturePath))
        {
            TEXImage image(texturePath.string());

            _width = image.GetWidth();
            _height = image.GetHeight();
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Item DBR % refers to bitmap % which does not exist. Width/height data will not be loaded.", path, texturePath);
        }
    }
}