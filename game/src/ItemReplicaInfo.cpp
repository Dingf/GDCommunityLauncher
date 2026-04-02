#include "ItemReplicaInfo.h"
#include "Log.h"

ItemReplicaInfo& ItemReplicaInfo::operator=(const ItemReplicaInfo& item)
{
    _itemName = item._itemName;
    _itemPrefix = item._itemPrefix;
    _itemSuffix = item._itemSuffix;
    _itemModifier = item._itemModifier;
    _itemIllusion = item._itemIllusion;
    _itemComponent = item._itemComponent;
    _itemCompletion = item._itemCompletion;
    _itemAugment = item._itemAugment;

    _itemID = item._itemID;
    _participantItemID = item._participantItemID;
    _itemSeed = item._itemSeed;
    _itemComponentSeed = item._itemComponentSeed;
    _unk3 = item._unk3;
    _itemAugmentSeed = item._itemAugmentSeed;
    _unk4 = item._unk4;
    _itemStackCount = item._itemStackCount;

    return *this;
}

void ItemReplicaInfo::Read(EncodedFileReader* reader)
{
    _itemID = 0;
    _participantItemID = 0;
    _itemName = reader->ReadString();
    _itemPrefix = reader->ReadString();
    _itemSuffix = reader->ReadString();
    _itemModifier = reader->ReadString();
    _itemIllusion = reader->ReadString();
    _itemSeed = reader->ReadInt32();
    _itemComponent = reader->ReadString();
    _itemCompletion = reader->ReadString();
    _itemComponentSeed = reader->ReadInt32();
    _itemAugment = reader->ReadString();
    _unk3 = reader->ReadInt32();
    _itemAugmentSeed = reader->ReadInt32();
    _unk4 = reader->ReadInt32();
    _itemStackCount = reader->ReadInt32();
}

void ItemReplicaInfo::Write(EncodedFileWriter* writer)
{
    writer->BufferString(_itemName);
    writer->BufferString(_itemPrefix);
    writer->BufferString(_itemSuffix);
    writer->BufferString(_itemModifier);
    writer->BufferString(_itemIllusion);
    writer->BufferInt32(_itemSeed);
    writer->BufferString(_itemComponent);
    writer->BufferString(_itemCompletion);
    writer->BufferInt32(_itemComponentSeed);
    writer->BufferString(_itemAugment);
    writer->BufferInt32(_unk3);
    writer->BufferInt32(_itemAugmentSeed);
    writer->BufferInt32(_unk4);
    writer->BufferInt32(_itemStackCount);
}

size_t ItemReplicaInfo::GetBufferSize() const
{
    size_t size = 56;
    size += _itemName.length();
    size += _itemPrefix.length();
    size += _itemSuffix.length();
    size += _itemModifier.length();
    size += _itemIllusion.length();
    size += _itemComponent.length();
    size += _itemCompletion.length();
    size += _itemAugment.length();
    return size;
}

void to_json(json& j, const ItemReplicaInfo& data)
{
    j = json
    {
        { "Name",              data._itemName },
        { "Prefix",            data._itemPrefix },
        { "Suffix",            data._itemSuffix },
        { "Modifier",          data._itemModifier },
        { "Illusion",          data._itemIllusion },
        { "ItemSeed",          data._itemSeed },
        { "Component",         data._itemComponent },
        { "Completion",        data._itemCompletion },
        { "ComponentSeed",     data._itemComponentSeed },
        { "Augment",           data._itemAugment },
        { "Unknown1",          data._unk3 },
        { "AugmentSeed",       data._itemAugmentSeed },
        { "Unknown2",          data._unk4 },
        { "StackCount",        data._itemStackCount },
        { "ParticipantItemID", data._participantItemID },
    };
}

void from_json(const json& j, ItemReplicaInfo& data)
{
    j.at("Name")         .get_to(data._itemName);
    j.at("Prefix")       .get_to(data._itemPrefix);
    j.at("Suffix")       .get_to(data._itemSuffix);
    j.at("Modifier")     .get_to(data._itemModifier);
    j.at("Illusion")     .get_to(data._itemIllusion);
    j.at("ItemSeed")     .get_to(data._itemSeed);
    j.at("Component")    .get_to(data._itemComponent);
    j.at("Completion")   .get_to(data._itemCompletion);
    j.at("ComponentSeed").get_to(data._itemComponentSeed);
    j.at("Augment")      .get_to(data._itemAugment);
    j.at("Unknown1")     .get_to(data._unk3);
    j.at("AugmentSeed")  .get_to(data._itemAugmentSeed);
    j.at("Unknown2")     .get_to(data._unk4);
    j.at("StackCount")   .get_to(data._itemStackCount);

    if (j.contains("ParticipantItemID"))
        j.at("ParticipantItemID").get_to(data._participantItemID);
}