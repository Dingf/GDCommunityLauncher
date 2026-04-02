#ifndef INC_GDCL_GAME_SHARED_STASH_H
#define INC_GDCL_GAME_SHARED_STASH_H

#include <filesystem>
#include "GDDataBlock.h"
#include "ItemContainer.h"
#include "Stash.h"

class SharedStash : public Stash
{
    public:
        // Header block, ID = 18, Version = 3,4,5
        struct SharedStashHeaderBlock : public GDDataBlock
        {
            SharedStashHeaderBlock() : GDDataBlock(0x12, 0x1C) {}

            friend void to_json(json& j, const SharedStashHeaderBlock& data);
            friend void from_json(const json& j, SharedStashHeaderBlock& data);

            uint32_t    _unk1;
            std::string _stashModName;
            uint8_t     _stashExpansions;
        };

        SharedStash() {}
        SharedStash(const std::filesystem::path& path) { ReadFromFile(path); };

        friend void to_json(json& j, const SharedStash& data);
        friend void from_json(const json& j, SharedStash& data);

        size_t GetBufferSize() const;

        bool ReadFromFile(const std::filesystem::path& path);
        bool ReadFromBuffer(const uint8_t* data, size_t size);
        bool WriteToFile(const std::filesystem::path& path);
        bool WriteToBuffer(uint8_t* data, size_t size);

        ItemContainerType GetContainerType() const { return ITEM_CONTAINER_SHARED_STASH; }

    private:
        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);

        SharedStashHeaderBlock _headerBlock;
};


#endif//INC_GDCL_GAME_SHARED_STASH_H