#ifndef INC_GDCL_GAME_REAGENTS_H
#define INC_GDCL_GAME_REAGENTS_H

#include <string>
#include <filesystem>
#include "GDDataBlock.h"
#include "JSON.h"

class Reagents
{
    public:
        struct ReagentsItemBlock : public GDDataBlock
        {
            ReagentsItemBlock() : GDDataBlock(0x00, 0x00) {}

            friend void to_json(json& j, const ReagentsItemBlock& data);
            friend void from_json(const json& j, ReagentsItemBlock& data);

            std::string _name;
            uint32_t    _count;
        };

        // Header block, ID = 20, Version = 1
        struct ReagentsHeaderBlock : public GDDataBlock
        {
            ReagentsHeaderBlock() : GDDataBlock(0x14, 0x01) {}

            friend void to_json(json& j, const ReagentsHeaderBlock& data);
            friend void from_json(const json& j, ReagentsHeaderBlock& data);

            std::string _modName;
            std::vector<std::unique_ptr<ReagentsItemBlock>> _items;
            uint32_t _unk1;
        }
        _headerBlock;

        Reagents() {}
        Reagents(const std::filesystem::path& path) { ReadFromFile(path); };

        friend void to_json(json& j, const Reagents& data);
        friend void from_json(const json& j, Reagents& data);

        size_t GetBufferSize() const;

        bool ReadFromFile(const std::filesystem::path& path);
        bool ReadFromBuffer(const uint8_t* data, size_t size);
        bool WriteToFile(const std::filesystem::path& path);
        bool WriteToBuffer(uint8_t* data, size_t size);

    private:
        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);
};

#endif//INC_GDCL_GAME_REAGENTS_H