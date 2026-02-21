#ifndef INC_GDCL_GAME_SKILL_H
#define INC_GDCL_GAME_SKILL_H

#include <memory>
#include <string>
#include "FileReader.h"
#include "JSON.h"

struct Skill
{
    public:
        virtual ~Skill() = 0;

        std::string _skillName;
        std::string _skillDevotionBind;
        std::string _skillDevotionTrigger;   // e.g. 25% chance when hit, 100% chance on crit, etc.
};

struct ClassSkill : public Skill
{
    public:
        ClassSkill() {}
        ClassSkill(EncodedFileReader* reader) { Read(reader); }

        friend void to_json(json& j, const ClassSkill& data);
        friend void from_json(const json& j, ClassSkill& data);

        void Read(EncodedFileReader* reader);

        uint32_t    _skillLevel;
        bool        _skillEnabled;
        uint32_t    _skillDevotionLevel;
        uint32_t    _skillExperience;
        uint32_t    _skillActive;
        uint8_t     _skillUnk1;
        uint8_t     _skillUnk2;
};

struct ItemSkill : public Skill
{
    public:
        ItemSkill() {}
        ItemSkill(EncodedFileReader* reader) { Read(reader); }

        friend void to_json(json& j, const ItemSkill& data);
        friend void from_json(const json& j, ItemSkill& data);

        void Read(EncodedFileReader* reader);

        uint32_t    _skillItemSlot;
        std::string _skillItemID;
};

#endif//INC_GDCL_GAME_SKILL_H