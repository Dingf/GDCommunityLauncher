#include "Skill.h"

Skill::~Skill() {}

void ClassSkill::Read(EncodedFileReader* reader)
{
    _skillName = reader->ReadString();
    _skillLevel = reader->ReadInt32();
    _skillEnabled = (reader->ReadInt8() != 0);
    _skillDevotionLevel = reader->ReadInt32();
    _skillExperience = reader->ReadInt32();
    _skillActive = reader->ReadInt32();
    _skillUnk1 = reader->ReadInt8();
    _skillUnk2 = reader->ReadInt8();
    _skillDevotionBind = reader->ReadString();
    _skillDevotionTrigger = reader->ReadString();
}

void to_json(json& j, const ClassSkill& data)
{
    j = 
    {
        { "Name",            data._skillName },
        { "Level",           data._skillLevel },
        { "Enabled",         data._skillEnabled },
        { "DevotionLevel",   data._skillDevotionLevel },
        { "Experience",      data._skillExperience },
        { "Active",          data._skillActive },
        { "Unknown1",        data._skillUnk1 },
        { "Unknown2",        data._skillUnk2 },
        { "DevotionBind",    data._skillDevotionBind },
        { "DevotionTrigger", data._skillDevotionTrigger },
    };
}

void from_json(const json& j, ClassSkill& data)
{
    j.at("Name")           .get_to(data._skillName);
    j.at("Level")          .get_to(data._skillLevel);
    j.at("Enabled")        .get_to(data._skillEnabled);
    j.at("DevotionLevel")  .get_to(data._skillDevotionLevel);
    j.at("Experience")     .get_to(data._skillExperience);
    j.at("Active")         .get_to(data._skillActive);
    j.at("Unknown1")       .get_to(data._skillUnk1);
    j.at("Unknown2")       .get_to(data._skillUnk2);
    j.at("DevotionBind")   .get_to(data._skillDevotionBind);
    j.at("DevotionTrigger").get_to(data._skillDevotionTrigger);
}

void ItemSkill::Read(EncodedFileReader* reader)
{
    _skillName = reader->ReadString();
    _skillDevotionBind = reader->ReadString();
    _skillDevotionTrigger = reader->ReadString();
    _skillItemSlot = reader->ReadInt32();
    _skillItemID = reader->ReadString();
}

void to_json(json& j, const ItemSkill& data)
{
    j = 
    {
        { "Name",            data._skillName },
        { "ItemSlot",        data._skillItemSlot },
        { "ItemID",          data._skillItemID },
        { "DevotionBind",    data._skillDevotionBind },
        { "DevotionTrigger", data._skillDevotionTrigger },
    };
}

void from_json(const json& j, ItemSkill& data)
{
    j.at("Name")           .get_to(data._skillName);
    j.at("ItemSlot")       .get_to(data._skillItemSlot);
    j.at("ItemID")         .get_to(data._skillItemID);
    j.at("DevotionBind")   .get_to(data._skillDevotionBind);
    j.at("DevotionTrigger").get_to(data._skillDevotionTrigger);
}