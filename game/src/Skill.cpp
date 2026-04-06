#include "Skill.h"

Skill::~Skill() {}

void ClassSkill::Read(EncodedFileReader* reader, uint32_t version)
{
    _version = version;
    _name = reader->ReadString();
    _level = reader->ReadInt32();
    _enabled = (reader->ReadInt8() != 0);

    if (version >= 7)
        _locked = (reader->ReadInt8() != 0);

    _devotionLevel = reader->ReadInt32();
    _experience = reader->ReadInt32();

    if (version <= 6)
    {
        _active = reader->ReadInt32();
        _locked = (reader->ReadInt8() != 0);
    }
    else if (version >= 7)
    {
        _subLevel = reader->ReadInt32();
        _active = reader->ReadInt8();
    }

    _transition = reader->ReadInt8();
    _devotionBind = reader->ReadString();
    _devotionTrigger = reader->ReadString();
}

void to_json(json& j, const ClassSkill& data)
{
    j = 
    {
        { "Name",            data._name },
        { "Level",           data._level },
        { "Enabled",         data._enabled },
        { "Locked",          data._locked },
        { "DevotionLevel",   data._devotionLevel },
        { "Experience",      data._experience },
        { "Sublevel",        data._subLevel },
        { "Active",          data._active },
        { "Transition",      data._transition },
        { "DevotionBind",    data._devotionBind },
        { "DevotionTrigger", data._devotionTrigger },
        { "BlockVersion",    data._version },
    };
}

void from_json(const json& j, ClassSkill& data)
{
    j.at("Name")           .get_to(data._name);
    j.at("Level")          .get_to(data._level);
    j.at("Enabled")        .get_to(data._enabled);
    j.at("Locked")         .get_to(data._locked);
    j.at("DevotionLevel")  .get_to(data._devotionLevel);
    j.at("Experience")     .get_to(data._experience);
    j.at("Sublevel")       .get_to(data._subLevel);
    j.at("Active")         .get_to(data._active);
    j.at("Transition")     .get_to(data._transition);
    j.at("DevotionBind")   .get_to(data._devotionBind);
    j.at("DevotionTrigger").get_to(data._devotionTrigger);
    j.at("BlockVersion")   .get_to(data._version);
}

void ItemSkill::Read(EncodedFileReader* reader)
{
    _name = reader->ReadString();
    _devotionBind = reader->ReadString();
    _devotionTrigger = reader->ReadString();
    _itemSlot = reader->ReadInt32();
    _itemID = reader->ReadString();
}

void to_json(json& j, const ItemSkill& data)
{
    j = 
    {
        { "Name",            data._name },
        { "ItemSlot",        data._itemSlot },
        { "ItemID",          data._itemID },
        { "DevotionBind",    data._devotionBind },
        { "DevotionTrigger", data._devotionTrigger },
    };
}

void from_json(const json& j, ItemSkill& data)
{
    j.at("Name")           .get_to(data._name);
    j.at("ItemSlot")       .get_to(data._itemSlot);
    j.at("ItemID")         .get_to(data._itemID);
    j.at("DevotionBind")   .get_to(data._devotionBind);
    j.at("DevotionTrigger").get_to(data._devotionTrigger);
}

void SubSkill::Read(EncodedFileReader* reader)
{
    _name = reader->ReadString();
    _devotionBind = reader->ReadString();
    _devotionTrigger = reader->ReadString();
    _parentSkill = reader->ReadString();
}

void to_json(json& j, const SubSkill& data)
{
    j =
    {
        { "Name",            data._name },
        { "DevotionBind",    data._devotionBind },
        { "DevotionTrigger", data._devotionTrigger },
        { "ParentSkill",     data._parentSkill },
    };
}

void from_json(const json& j, SubSkill& data)
{
    j.at("Name")           .get_to(data._name);
    j.at("DevotionBind")   .get_to(data._devotionBind);
    j.at("DevotionTrigger").get_to(data._devotionTrigger);
    j.at("ParentSkill")    .get_to(data._parentSkill);
}