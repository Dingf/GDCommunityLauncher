#ifndef INC_GDCL_EXTRACTOR_TEX_IMAGE_H
#define INC_GDCL_EXTRACTOR_TEX_IMAGE_H

#include <string>
#include <filesystem>
#include "FileReader.h"
#include "DDSImage.h"

// = "TEX"
constexpr uint32_t TEX_MAGIC = 0x584554;

class TEXImage : public DDSImage
{
    public:
        TEXImage(const std::filesystem::path& path) { Load(path); }

        bool Load(const std::filesystem::path& path);
    private:
        void LoadTEXHeader(FileReader* reader);

        struct TEXHeader
        {
            uint32_t _magic;
            uint8_t  _version;
            uint32_t _unk1;
            uint32_t _size;
        }
        _TEXHeader;
};


#endif//INC_GDCL_EXTRACTOR_TEX_IMAGE_H