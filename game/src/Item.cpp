#include "Item.h"

Item::Item(EncodedFileReader* reader)
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