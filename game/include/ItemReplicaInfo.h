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
        ItemReplicaInfo(EncodedFileReader* reader, uint32_t version) { Read(reader, version); }
        ItemReplicaInfo(const ItemReplicaInfo& item) { *this = item; }

        ItemReplicaInfo& operator=(const ItemReplicaInfo& item);

        friend void to_json(json& j, const ItemReplicaInfo& data);
        friend void from_json(const json& j, ItemReplicaInfo& data);

        void Read(EncodedFileReader* reader, uint32_t version);
        void Write(EncodedFileWriter* writer, uint32_t version);

        size_t GetBufferSize() const;

        uint32_t    _itemID;
        std::string _name;
        std::string _prefix;
        std::string _suffix;
        uint32_t    _seed;
        union                   // TODO: Check to make sure that using that using this unknown value doesn't break anything
        {
            uint32_t _unk1;
            uint32_t _participantItemID;
        };
        std::string _modifier;
        std::string _component;
        std::string _completion;
        uint32_t    _componentSeed;
        uint32_t    _unk2;
        std::string _augment;
        uint32_t    _augmentLevel;
        uint32_t    _augmentSeed;
        std::string _illusion;
        uint32_t    _unk4;      // _itemUnk2, stored in item data
        uint32_t    _unk5;
        uint64_t    _unk6;
        uint32_t    _unk7;
        uint32_t    _stackCount;
        uint32_t    _unk8;
        uint32_t    _unk9;
        uint32_t    _unk10;
        uint32_t    _unk11;

        // TODO: The following are new fields in the item, figure out where they fit in the ItemReplicaInfo struct:
        //   std::string _ascendant
        //   std::string _ascendant2H
        //   uint32_t    _rerollsUsed
};

#endif//INC_GDCL_GAME_ITEM_REPLICA_INFO_H