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
        ItemReplicaInfo() : _participantItemID(0) {}
        ItemReplicaInfo(EncodedFileReader* reader, uint32_t version) { Read(reader, version); }
        ItemReplicaInfo(const ItemReplicaInfo& item) { *this = item; }

        ItemReplicaInfo& operator=(const ItemReplicaInfo& item);

        friend void to_json(json& j, const ItemReplicaInfo& data);
        friend void from_json(const json& j, ItemReplicaInfo& data);

        void Read(EncodedFileReader* reader, uint32_t version);
        void Write(EncodedFileWriter* writer, uint32_t version);

        size_t GetBufferSize() const;

        uint64_t    _itemID;        // 0
        std::string _name;          // 8
        std::string _prefix;        // 28
        std::string _suffix;        // 48
        uint64_t    _seed;          // 68
        std::string _modifier;      // 70
        std::string _component;     // 90
        std::string _completion;    // B0
        uint64_t    _componentSeed; // D0
        std::string _augment;       // D8
        uint32_t    _augmentLevel;  // F8
        uint32_t    _augmentSeed;   // FC
        std::string _illusion;      // 100
        std::string _ascendant;     // 120
        std::string _ascendant2H;   // 140
        uint32_t    _unk1;          // 160
        uint32_t    _unk2;          // 164
        uint32_t    _unk3;          // 168
        uint32_t    _unk4;          // 16C
        uint32_t    _unk5;          // 170
        uint32_t    _unk6;          // 174
        uint32_t    _stackCount;    // 178
        uint32_t    _rerollsUsed;   // 17C
        uint32_t    _unk7;          // 180
        uint32_t    _unk8;          // 184
        uint32_t    _participantItemID;
};

#endif//INC_GDCL_GAME_ITEM_REPLICA_INFO_H