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

    SetState(true);
}

void ItemSkill::Read(EncodedFileReader* reader)
{
    _skillName = reader->ReadString();
    _skillDevotionBind = reader->ReadString();
    _skillDevotionTrigger = reader->ReadString();
    _skillItemSlot = reader->ReadInt32();
    _skillItemID = reader->ReadString();

    SetState(true);
}