#ifndef INC_GDCL_DLL_GAME_API_ITEM_REPLICA_INFO_H
#define INC_GDCL_DLL_GAME_API_ITEM_REPLICA_INFO_H

#include <stdint.h>
#include <string>
#include "Item.h"

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GET_ITEM_REPLICA_INFO[] = "?GetItemReplicaInfo@Item@GAME@@UEBAXAEAUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_CREATE_ITEM[] = "?CreateItem@Item@GAME@@SAPEAV12@AEBUItemReplicaInfo@2@@Z";
#else
constexpr char GAPI_NAME_GET_ITEM_REPLICA_INFO[] = "?GetItemReplicaInfo@Item@GAME@@UBEXAAUItemReplicaInfo@2@@Z";
constexpr char GAPI_NAME_CREATE_ITEM[] = "?CreateItem@Item@GAME@@SAPAV12@ABUItemReplicaInfo@2@@Z";
#endif

struct ItemReplicaInfo
{
    ItemReplicaInfo() {}
    ItemReplicaInfo(const Item& item)
    {
        *this = item;
    }

    ItemReplicaInfo& operator=(const Item& item)
    {
        _itemName = item._itemName;
        _itemPrefix = item._itemPrefix;
        _itemSuffix = item._itemSuffix;
        _itemModifier = item._itemModifier;
        _itemIllusion = item._itemIllusion;
        _itemComponent = item._itemComponent;
        _itemCompletion = item._itemCompletion;
        _itemAugment = item._itemAugment;
        _itemSeed = item._itemSeed;
        _itemComponentSeed = item._itemComponentSeed;
        _unk3 = item._itemUnk1;
        _itemAugmentSeed = item._itemAugmentSeed;
        _unk4 = item._itemUnk2;
        _itemStackCount = item._itemStackCount;

        _unk1 = 0;
        _unk2 = 0;
        _unk5 = 0;
        _unk6 = 0;
        _unk7 = 0;

        return *this;
    }

    operator Item() const
    {
        Item item;

        item._itemName = _itemName;
        item._itemPrefix = _itemPrefix;
        item._itemSuffix = _itemSuffix;
        item._itemModifier = _itemModifier;
        item._itemIllusion = _itemIllusion;
        item._itemComponent = _itemComponent;
        item._itemCompletion = _itemCompletion;
        item._itemAugment = _itemAugment;
        item._itemSeed = _itemSeed;
        item._itemComponentSeed = _itemComponentSeed;
        item._itemUnk1 = _unk3;
        item._itemAugmentSeed = _itemAugmentSeed;
        item._itemUnk2 = _unk4;
        item._itemStackCount = _itemStackCount;

        return item;
    }

    uint32_t    _itemID;
    std::string _itemName;
    std::string _itemPrefix;
    std::string _itemSuffix;
    uint32_t    _itemSeed;
    uint32_t    _unk1;
    std::string _itemModifier;
    std::string _itemComponent;
    std::string _itemCompletion;
    uint32_t    _itemComponentSeed;
    uint32_t    _unk2;
    std::string _itemAugment;
    uint32_t    _unk3;      // _itemUnk1
    uint32_t    _itemAugmentSeed;
    std::string _itemIllusion;
    uint32_t    _unk4;      // _itemUnk2
    uint32_t    _unk5;
    uint64_t    _unk6;
    uint32_t    _unk7;
    uint32_t    _itemStackCount;
};

void GetItemReplicaInfo(void* item, ItemReplicaInfo& info);
void* CreateItem(const ItemReplicaInfo& info);

}

#endif//INC_GDCL_DLL_GAME_API_ITEM_REPLICA_INFO_H