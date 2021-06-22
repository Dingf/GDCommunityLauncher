#ifndef INC_GDCL_GAME_ITEM_H
#define INC_GDCL_GAME_ITEM_H

#include <memory>
#include <string>
#include "FileReader.h"

struct Item
{
    public:
        Item(EncodedFileReader* reader);

        std::string _itemName;
        std::string _itemPrefix;
        std::string _itemSuffix;
        std::string _itemModifier;
        std::string _itemIllusion;
        std::string _itemComponent;
        std::string _itemCompletion;
        std::string _itemAugment;

        uint32_t _itemSeed;
        uint32_t _itemComponentSeed;
        uint32_t _itemUnk1;
        uint32_t _itemAugmentSeed;
        uint32_t _itemUnk2;
        uint32_t _itemStackCount;
};

#endif//INC_GDCL_GAME_ITEM_H