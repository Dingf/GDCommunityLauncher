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

        std::string _name;
        std::string _devotionBind;
        std::string _devotionTrigger;   // e.g. 25% chance when hit, 100% chance on crit, etc.
};

struct ClassSkill : public Skill
{
    public:
        ClassSkill() {}
        ClassSkill(EncodedFileReader* reader, uint32_t version) { Read(reader, version); }

        friend void to_json(json& j, const ClassSkill& data);
        friend void from_json(const json& j, ClassSkill& data);

        void Read(EncodedFileReader* reader, uint32_t version);

        bool        _enabled;
        bool        _locked;
        uint8_t     _transition;
        uint32_t    _level;
        uint32_t    _subLevel;
        uint32_t    _devotionLevel;
        uint32_t    _experience;
        uint32_t    _active;
        uint32_t    _version;
};

struct ItemSkill : public Skill
{
    public:
        ItemSkill() {}
        ItemSkill(EncodedFileReader* reader) { Read(reader); }

        friend void to_json(json& j, const ItemSkill& data);
        friend void from_json(const json& j, ItemSkill& data);

        void Read(EncodedFileReader* reader);

        uint32_t    _itemSlot;
        std::string _itemID;
};

struct SubSkill : public Skill
{
    public:
        SubSkill() {}
        SubSkill(EncodedFileReader* reader) { Read(reader); }

        friend void to_json(json& j, const SubSkill& data);
        friend void from_json(const json& j, SubSkill& data);

        void Read(EncodedFileReader* reader);

        std::string _parentSkill;
};

#endif//INC_GDCL_GAME_SKILL_H