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
    _unk4 = item._unk4;
    _stackCount = item._stackCount;

    /*_ascendant = item._ascendant;
    _ascendant2H = item._ascendant2H;
    _rerollsUsed = item._rerollsUsed;*/

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
        // TODO: Use the new fields after adding them to ItemReplicaInfo
        reader->ReadString();
        reader->ReadString();
        //_ascendant = reader->ReadString();
        //_ascendant2H = reader->ReadString();
    }
    _unk4 = reader->ReadInt32();
    _stackCount = reader->ReadInt32();
    if (version >= 8)
    {
        // TODO: Use the new fields after adding them to ItemReplicaInfo
        reader->ReadInt32();
        //_rerollsUsed = reader->ReadInt32();
    }
}

void ItemReplicaInfo::Write(EncodedFileWriter* writer, uint32_t version)
{
    writer->BufferString(_name);
    writer->BufferString(_prefix);
    writer->BufferString(_suffix);
    writer->BufferString(_modifier);
    writer->BufferString(_illusion);
    writer->BufferInt32(_seed);
    writer->BufferString(_component);
    writer->BufferString(_completion);
    writer->BufferInt32(_componentSeed);
    writer->BufferString(_augment);
    writer->BufferInt32(_augmentLevel);
    writer->BufferInt32(_augmentSeed);
    if (version >= 8)
    {
        // TODO: Use the new fields after adding them to ItemReplicaInfo
        writer->BufferString("");
        writer->BufferString("");
        //writer->BufferString(_ascendant);
        //writer->BufferString(_ascendant2H);
    }
    writer->BufferInt32(_unk4);
    writer->BufferInt32(_stackCount);
    if (version >= 8)
    {
        // TODO: Use the new fields after adding them to ItemReplicaInfo
        writer->BufferInt32(0);
        //writer->BufferInt32(_rerollsUsed);
    }
}

size_t ItemReplicaInfo::GetBufferSize() const
{
    size_t size = 68;
    size += _name.length();
    size += _prefix.length();
    size += _suffix.length();
    size += _modifier.length();
    size += _illusion.length();
    size += _component.length();
    size += _completion.length();
    size += _augment.length();
    //size += _ascendant.length();
    //size += _ascendant2H.length();
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
        { "Unknown2",          data._unk4 },
        { "StackCount",        data._stackCount },
        /*
        // TODO: Use the new fields after adding them to ItemReplicaInfo
        { "Ascendant",         data._ascendant },
        { "Ascendant2H",       data._ascendant2H },
        { "RerollsUsed",       data._rerollsUsed },
        */
        { "ParticipantItemID", data._participantItemID },
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
    j.at("Unknown2")     .get_to(data._unk4);
    j.at("StackCount")   .get_to(data._stackCount);

    /*
    // TODO: Use the new fields after adding them to ItemReplicaInfo
    j.at("Ascendant")    .get_to(data._ascendant);
    j.at("Ascendant2H")  .get_to(data._ascendant2H);
    j.at("RerollsUsed")  .get_to(data._rerollsUsed);
    */

    if (j.contains("ParticipantItemID"))
        j.at("ParticipantItemID").get_to(data._participantItemID);
}