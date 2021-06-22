#ifndef INC_GDCL_GAME_STASH_H
#define INC_GDCL_GAME_STASH_H

#include <memory>
#include <vector>
#include "Object.h"
#include "GDDataBlock.h"
#include "ItemContainer.h"

class Stash : public Object
{
    public:
        virtual ~Stash() = 0;

        virtual ItemContainerType GetContainerType() const = 0;

        bool IsHardcore() const { return _isHardcore; }
        void SetHardcore(bool hardcore) { _isHardcore = hardcore; }

        void ReadStashTab(EncodedFileReader* reader);

    protected:
        class StashTab : public ItemContainer
        {
            friend class Stash;

            public:
                ItemContainerType GetContainerType() const { return _parent.GetContainerType(); }

            private:
                StashTab(const Stash& stash, uint32_t width, uint32_t height) : ItemContainer(width, height), _parent(stash) {}

                const Stash& _parent;
        };

        struct StashTabBlock : public GDDataBlock
        {
            StashTabBlock() : GDDataBlock(0x00, 0x00) {}
            ~StashTabBlock() {}

            std::unique_ptr<StashTab> _stashTab;
        };

        bool _isHardcore;
        std::vector<std::unique_ptr<StashTabBlock>> _stashTabs;
};

#endif//INC_GDCL_GAME_STASH_H