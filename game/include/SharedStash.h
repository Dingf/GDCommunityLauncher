#ifndef INC_GDCL_GAME_SHARED_STASH_H
#define INC_GDCL_GAME_SHARED_STASH_H

#include <filesystem>
#include "FileData.h"
#include "GDDataBlock.h"
#include "ItemContainer.h"
#include "Stash.h"

class SharedStash : public Stash
{
    public:
        SharedStash() {}
        SharedStash(const std::filesystem::path& path) { ReadFromFile(path); };

        size_t GetBufferSize() const;

        bool ReadFromFile(const std::filesystem::path& path);
        bool ReadFromBuffer(uint8_t* data, size_t size);
        bool WriteToFile(const std::filesystem::path& path);
        bool WriteToBuffer(uint8_t* data, size_t size);

        ItemContainerType GetContainerType() const { return ITEM_CONTAINER_SHARED_STASH; }

    private:
        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);

        // Header block, ID = 18, Version = 3,4,5
        struct SharedStashHeaderBlock : public GDDataBlock
        {
            SharedStashHeaderBlock() : GDDataBlock(0x12, 0x1C) {}

            web::json::value ToJSON() const;

            uint32_t    _unk1;
            std::string _stashModName;
            uint8_t     _stashExpansions;
        }
        _headerBlock;
};


#endif//INC_GDCL_GAME_SHARED_STASH_H