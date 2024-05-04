#include "Item.h"
#include "Log.h"

Item::Item(const web::json::value& obj)
{
    if (obj.has_integer_field(U("participantItemId")))
        _itemID = obj.at(U("participantItemId")).as_integer();

    _itemName = JSONString(obj.at(U("name")).serialize());
    _itemPrefix = JSONString(obj.at(U("prefix")).serialize());
    _itemSuffix = JSONString(obj.at(U("suffix")).serialize());
    _itemModifier = JSONString(obj.at(U("modifier")).serialize());
    _itemIllusion = JSONString(obj.at(U("illusion")).serialize());
    _itemSeed = obj.at(U("itemSeed")).as_integer();
    _itemComponent = JSONString(obj.at(U("component")).serialize());
    _itemCompletion = JSONString(obj.at(U("completion")).serialize());
    _itemComponentSeed = obj.at(U("componentSeed")).as_integer();
    _itemAugment = JSONString(obj.at(U("augment")).serialize());
    _itemUnk1 = 0;
    _itemAugmentSeed = obj.at(U("augmentSeed")).as_integer();
    _itemUnk2 = 0;
    _itemStackCount = obj.at(U("stackCount")).as_integer();
}

Item& Item::operator=(const Item& item)
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
    _itemSeed = item._itemSeed;
    _itemComponentSeed = item._itemComponentSeed;
    _itemUnk1 = item._itemUnk1;
    _itemAugmentSeed = item._itemAugmentSeed;
    _itemUnk2 = item._itemUnk2;
    _itemStackCount = item._itemStackCount;

    return *this;
}

void Item::Read(EncodedFileReader* reader)
{
    _itemID = 0;
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
    _itemUnk1 = reader->ReadInt32();
    _itemAugmentSeed = reader->ReadInt32();
    _itemUnk2 = reader->ReadInt32();
    _itemStackCount = reader->ReadInt32();
}

void Item::Write(EncodedFileWriter* writer)
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
    writer->BufferInt32(_itemUnk1);
    writer->BufferInt32(_itemAugmentSeed);
    writer->BufferInt32(_itemUnk2);
    writer->BufferInt32(_itemStackCount);
}

size_t Item::GetBufferSize() const
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

web::json::value Item::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("name")] = JSONString(_itemName);
    obj[U("prefix")] = JSONString(_itemPrefix);
    obj[U("suffix")] = JSONString(_itemSuffix);
    obj[U("modifier")] = JSONString(_itemModifier);
    obj[U("illusion")] = JSONString(_itemIllusion);
    obj[U("itemSeed")] = _itemSeed;
    obj[U("component")] = JSONString(_itemComponent);
    obj[U("completion")] = JSONString(_itemCompletion);
    obj[U("componentSeed")] = _itemComponentSeed;
    obj[U("augment")] = JSONString(_itemAugment);
    obj[U("unknown1")] = _itemUnk1;
    obj[U("augmentSeed")] = _itemAugmentSeed;
    obj[U("unknown2")] = _itemUnk2;
    obj[U("stackCount")] = _itemStackCount;

    return obj;
}