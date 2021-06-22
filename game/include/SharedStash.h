#ifndef INC_GDCL_GAME_SHARED_STASH_H
#define INC_GDCL_GAME_SHARED_STASH_H

#include <filesystem>
#include "ItemContainer.h"
#include "Stash.h"

class SharedStash : public Stash
{
    public:
        SharedStash() {}
        SharedStash(const std::filesystem::path& path) { ReadFromFile(path); };

        bool ReadFromFile(const std::filesystem::path& path);

        ItemContainerType GetContainerType() const { return ITEM_CONTAINER_SHARED_STASH; }

    private:
        void ReadSharedStashData(EncodedFileReader* reader);
};


#endif//INC_GDCL_GAME_SHARED_STASH_H