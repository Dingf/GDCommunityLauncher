#ifndef INC_GDCL_GAME_ITEM_REPLICA_INFO_H
#define INC_GDCL_GAME_ITEM_REPLICA_INFO_H

#include <memory>
#include <string>
#include "FileReader.h"
#include "FileWriter.h"
#include "JSON.h"

struct ItemReplicaInfo
{
    public:
        ItemReplicaInfo() {}
        ItemReplicaInfo(EncodedFileReader* reader) { Read(reader); }
        ItemReplicaInfo(const ItemReplicaInfo& item) { *this = item; }

        ItemReplicaInfo& operator=(const ItemReplicaInfo& item);

        friend void to_json(json& j, const ItemReplicaInfo& data);
        friend void from_json(const json& j, ItemReplicaInfo& data);

        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);

        size_t GetBufferSize() const;

        uint32_t    _itemID;
        std::string _itemName;
        std::string _itemPrefix;
        std::string _itemSuffix;
        uint32_t    _itemSeed;
        union                   // This appears to be unused, but check to make sure that using this doesn't break anything
        {
            uint32_t _unk1;
            uint32_t _participantItemID;
        };
        std::string _itemModifier;
        std::string _itemComponent;
        std::string _itemCompletion;
        uint32_t    _itemComponentSeed;
        uint32_t    _unk2;
        std::string _itemAugment;
        uint32_t    _unk3;      // _itemUnk1, stored in item data
        uint32_t    _itemAugmentSeed;
        std::string _itemIllusion;
        uint32_t    _unk4;      // _itemUnk2, stored in item data
        uint32_t    _unk5;
        uint64_t    _unk6;
        uint32_t    _unk7;
        uint32_t    _itemStackCount;
        uint32_t    _unk8;
        uint32_t    _unk9;
        uint32_t    _unk10;
        uint32_t    _unk11;
};

#endif//INC_GDCL_GAME_ITEM_REPLICA_INFO_H