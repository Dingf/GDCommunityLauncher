#ifndef INC_GDCL_GAME_STASH_H
#define INC_GDCL_GAME_STASH_H

#include <memory>
#include <vector>
#include "FileData.h"
#include "JSONObject.h"
#include "GDDataBlock.h"
#include "ItemContainer.h"

class Stash : public FileData, public JSONObject
{
    public:
        class StashTab : public ItemContainer
        {
            friend class Stash;

            public:
                ItemContainerType GetContainerType() const { return _parent.GetContainerType(); }

            private:
                StashTab(const Stash& stash, uint32_t width, uint32_t height) : ItemContainer(width, height), _parent(stash) {}

                const Stash& _parent;
        };

        virtual ~Stash() = 0;

        virtual size_t GetBufferSize() const;

        virtual web::json::value ToJSON();

        virtual ItemContainerType GetContainerType() const = 0;

        bool IsHardcore() const { return _isHardcore; }
        void SetHardcore(bool hardcore) { _isHardcore = hardcore; }

        void ReadStashTabs(EncodedFileReader* reader, uint32_t count);
        void WriteStashTabs(EncodedFileWriter* writer);

        uint32_t GetTabCount() const { return _stashTabs.size(); }

        StashTab* GetStashTab(uint32_t index);

    protected:
        struct StashTabBlock : public GDDataBlock
        {
            StashTabBlock() : GDDataBlock(0x00, 0x00) {}
            ~StashTabBlock() {}

            web::json::value ToJSON();

            std::unique_ptr<StashTab> _stashTab;
        };

        bool _isHardcore;
        std::vector<std::unique_ptr<StashTabBlock>> _stashTabs;
};

#endif//INC_GDCL_GAME_STASH_H