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

web::json::value ClassSkill::ToJSON()
{
    web::json::value obj = web::json::value::object();

    obj[U("Name")] = JSONString(_skillName);
    obj[U("Level")] = _skillLevel;
    obj[U("Enabled")] = _skillEnabled;
    obj[U("DevotionLevel")] = _skillDevotionLevel;
    obj[U("Experience")] = _skillExperience;
    obj[U("Active")] = _skillActive;
    obj[U("Unknown1")] = _skillUnk1;
    obj[U("Unknown2")] = _skillUnk2;
    obj[U("DevotionBind")] = JSONString(_skillDevotionBind);
    obj[U("DevotionTrigger")] = JSONString(_skillDevotionTrigger);

    return obj;
}

void ItemSkill::Read(EncodedFileReader* reader)
{
    _skillName = reader->ReadString();
    _skillDevotionBind = reader->ReadString();
    _skillDevotionTrigger = reader->ReadString();
    _skillItemSlot = reader->ReadInt32();
    _skillItemID = reader->ReadString();
}

web::json::value ItemSkill::ToJSON()
{
    web::json::value obj = web::json::value::object();

    obj[U("Name")] = JSONString(_skillName);
    obj[U("ItemSlot")] = _skillItemSlot;
    obj[U("ItemID")] = JSONString(_skillItemID);
    obj[U("DevotionBind")] = JSONString(_skillDevotionBind);
    obj[U("DevotionTrigger")] = JSONString(_skillDevotionTrigger);

    return obj;
}