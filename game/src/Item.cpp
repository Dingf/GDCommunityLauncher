#include "Item.h"

void Item::Read(EncodedFileReader* reader)
{
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

web::json::value Item::ToJSON()
{
    web::json::value obj = web::json::value::object();

    obj[U("Name")] = JSONString(_itemName);
    obj[U("Prefix")] = JSONString(_itemPrefix);
    obj[U("Suffix")] = JSONString(_itemSuffix);
    obj[U("Modifier")] = JSONString(_itemModifier);
    obj[U("Illusion")] = JSONString(_itemIllusion);
    obj[U("ItemSeed")] = _itemSeed;
    obj[U("Component")] = JSONString(_itemComponent);
    obj[U("Completion")] = JSONString(_itemCompletion);
    obj[U("ComponentSeed")] = _itemComponentSeed;
    obj[U("Augment")] = JSONString(_itemAugment);
    obj[U("Unknown1")] = _itemUnk1;
    obj[U("AugmentSeed")] = _itemAugmentSeed;
    obj[U("Unknown2")] = _itemUnk2;
    obj[U("StackCount")] = _itemStackCount;

    return obj;
}