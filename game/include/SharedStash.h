#ifndef INC_GDCL_GAME_SHARED_STASH_H
#define INC_GDCL_GAME_SHARED_STASH_H

#include "ItemContainer.h"
#include "Stash.h"

class SharedStash : public Stash
{
    public:
        SharedStash(EncodedFileReader* reader);

        ItemContainerType GetContainerType() const { return ITEM_CONTAINER_SHARED_STASH; }

    private:
        SharedStash() {}
};


#endif//INC_GDCL_GAME_SHARED_STASH_H