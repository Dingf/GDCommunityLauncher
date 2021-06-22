#include "Skill.h"

Skill::~Skill() {}

ClassSkill::ClassSkill(EncodedFileReader* reader)
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

ItemSkill::ItemSkill(EncodedFileReader* reader)
{
    _skillName = reader->ReadString();
    _skillDevotionBind = reader->ReadString();
    _skillDevotionTrigger = reader->ReadString();
    _skillItemSlot = reader->ReadInt32();
    _skillItemID = reader->ReadString();
}