#include "ItemReplicaInfo.h"
#include "Log.h"

ItemReplicaInfo& ItemReplicaInfo::operator=(const ItemReplicaInfo& item)
{
    _name = item._name;
    _prefix = item._prefix;
    _suffix = item._suffix;
    _modifier = item._modifier;
    _illusion = item._illusion;
    _component = item._component;
    _completion = item._completion;
    _augment = item._augment;
    _itemID = item._itemID;
    _participantItemID = item._participantItemID;
    _seed = item._seed;
    _componentSeed = item._componentSeed;
    _augmentLevel = item._augmentLevel;
    _augmentSeed = item._augmentSeed;
    _unk1 = item._unk1;
    _stackCount = item._stackCount;
    _ascendant = item._ascendant;
    _ascendant2H = item._ascendant2H;
    _rerolls = item._rerolls;
    _affixRerolls = item._affixRerolls;
    _version = item._version;
    return *this;
}

void ItemReplicaInfo::Read(EncodedFileReader* reader, uint32_t version)
{
    _itemID = 0;
    _participantItemID = 0;
    _name = reader->ReadString();
    _prefix = reader->ReadString();
    _suffix = reader->ReadString();
    _modifier = reader->ReadString();
    _illusion = reader->ReadString();
    _seed = reader->ReadInt32();
    _component = reader->ReadString();
    _completion = reader->ReadString();
    _componentSeed = reader->ReadInt32();
    _augment = reader->ReadString();
    _augmentLevel = reader->ReadInt32();
    _augmentSeed = reader->ReadInt32();
    if (version >= 8)
    {
        _ascendant = reader->ReadString();
        _ascendant2H = reader->ReadString();
    }
    _unk1 = reader->ReadInt32();
    _stackCount = reader->ReadInt32();
    if (version >= 8)
    {
        _rerolls = reader->ReadInt32();
    }
    if (version >= 11)
    {
        _affixRerolls = reader->ReadInt32();
    }
    _version = version;
}

void ItemReplicaInfo::Write(EncodedFileWriter* writer)
{
    writer->BufferString(_name);
    writer->BufferString(_prefix);
    writer->BufferString(_suffix);
    writer->BufferString(_modifier);
    writer->BufferString(_illusion);
    writer->BufferInt32((uint32_t)_seed);
    writer->BufferString(_component);
    writer->BufferString(_completion);
    writer->BufferInt32((uint32_t)_componentSeed);
    writer->BufferString(_augment);
    writer->BufferInt32(_augmentLevel);
    writer->BufferInt32(_augmentSeed);
    if (_version >= 8)
    {
        writer->BufferString(_ascendant);
        writer->BufferString(_ascendant2H);
    }
    writer->BufferInt32(_unk1);
    writer->BufferInt32(_stackCount);
    if (_version >= 8)
    {
        writer->BufferInt32(_rerolls);
    }
    if (_version >= 11)
    {
        writer->BufferInt32(_affixRerolls);
    }
}

size_t ItemReplicaInfo::GetBufferSize() const
{
    size_t size = 56;
    if (_version >= 8)
        size += 12;
    if (_version >= 11)
        size += 4;
    size += _name.length();
    size += _prefix.length();
    size += _suffix.length();
    size += _modifier.length();
    size += _illusion.length();
    size += _component.length();
    size += _completion.length();
    size += _augment.length();
    if (_version >= 8)
    {
        size += _ascendant.length();
        size += _ascendant2H.length();
    }
    return size;
}

void to_json(json& j, const ItemReplicaInfo& data)
{
    j = json
    {
        { "Name",              data._name },
        { "Prefix",            data._prefix },
        { "Suffix",            data._suffix },
        { "Modifier",          data._modifier },
        { "Illusion",          data._illusion },
        { "ItemSeed",          data._seed },
        { "Component",         data._component },
        { "Completion",        data._completion },
        { "ComponentSeed",     data._componentSeed },
        { "Augment",           data._augment },
        { "AugmentLevel",      data._augmentLevel },
        { "AugmentSeed",       data._augmentSeed },
        { "StackCount",        data._stackCount },
        { "Ascendant",         data._ascendant },
        { "Ascendant2h",       data._ascendant2H },
        { "RerollsUsed",       data._rerolls },
        { "AffixRerollsUsed",  data._affixRerolls },
        { "ParticipantItemId", data._participantItemID },
        { "Version",           data._version },
    };
}

void from_json(const json& j, ItemReplicaInfo& data)
{
    j.at("Name")         .get_to(data._name);
    j.at("Prefix")       .get_to(data._prefix);
    j.at("Suffix")       .get_to(data._suffix);
    j.at("Modifier")     .get_to(data._modifier);
    j.at("Illusion")     .get_to(data._illusion);
    j.at("ItemSeed")     .get_to(data._seed);
    j.at("Component")    .get_to(data._component);
    j.at("Completion")   .get_to(data._completion);
    j.at("ComponentSeed").get_to(data._componentSeed);
    j.at("Augment")      .get_to(data._augment);
    j.at("AugmentLevel") .get_to(data._augmentLevel);
    j.at("AugmentSeed")  .get_to(data._augmentSeed);
    j.at("StackCount")   .get_to(data._stackCount);

    // Backwards compatability with old items that don't have these fields
    if ((j.contains("Ascendant")) && (!j.at("Ascendant").is_null()))
        j.at("Ascendant").get_to(data._ascendant);
    if ((j.contains("Ascendant2h")) && (!j.at("Ascendant2h").is_null()))
        j.at("Ascendant2h").get_to(data._ascendant2H);
    if ((j.contains("RerollsUsed")) && (!j.at("RerollsUsed").is_null()))
        j.at("RerollsUsed").get_to(data._rerolls);
    if ((j.contains("AffixRerollsUsed")) && (!j.at("AffixRerollsUsed").is_null()))
        j.at("AffixRerollsUsed").get_to(data._affixRerolls);
    if ((j.contains("ParticipantItemId")) && (!j.at("ParticipantItemId").is_null()))
        j.at("ParticipantItemId").get_to(data._participantItemID);
    if ((j.contains("Version")) && (!j.at("Version").is_null()))
        j.at("Version").get_to(data._version);
}