#ifndef INC_GDCL_GAME_STASH_H
#define INC_GDCL_GAME_STASH_H

#include <memory>
#include <vector>
#include "FileData.h"
#include "JSON.h"
#include "GDDataBlock.h"
#include "ItemContainer.h"

class Stash : public FileData
{
    public:
        class StashTab : public ItemContainer
        {
            friend class Stash;

            public:
                StashTab(ItemContainerType type, uint32_t width, uint32_t height) : ItemContainer(width, height), _type(type) {}

                ItemContainerType GetContainerType() const { return _type; }

            private:
                ItemContainerType _type;
        };

        struct StashTabBlock : public GDDataBlock
        {
            StashTabBlock() : GDDataBlock(0x00, 0x00) {}
            ~StashTabBlock() {}

            friend void to_json(json& j, const StashTabBlock& data);
            friend void from_json(const json& j, StashTabBlock& data);

            std::unique_ptr<StashTab> _stashTab;
        };

        virtual ~Stash() = 0;

        virtual size_t GetBufferSize() const;

        friend void to_json(json& j, const Stash& data);
        friend void from_json(const json& j, Stash& data);

        virtual ItemContainerType GetContainerType() const = 0;

        bool IsHardcore() const { return _isHardcore; }
        void SetHardcore(bool hardcore) { _isHardcore = hardcore; }

        void ReadStashTabs(EncodedFileReader* reader, size_t count);
        void WriteStashTabs(EncodedFileWriter* writer);

        size_t GetTabCount() const { return _stashTabs.size(); }

        StashTab* GetStashTab(size_t index);

    protected:
        bool _isHardcore;
        std::vector<std::unique_ptr<StashTabBlock>> _stashTabs;
};

#endif//INC_GDCL_GAME_STASH_H