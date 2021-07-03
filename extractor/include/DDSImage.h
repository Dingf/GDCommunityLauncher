#ifndef INC_GDCL_EXTRACTOR_DDS_IMAGE_H
#define INC_GDCL_EXTRACTOR_DDS_IMAGE_H

#include <string>
#include <vector>
#include "FileReader.h"

// Usually this is supposed to be "DDS " (0x20534444) but in the TEX files
// the magic number is "DDSR" for some reason
constexpr uint32_t DDSR_MAGIC = 0x52534444;

constexpr uint32_t DDS_MAGIC = 0x20534444;

enum DDSFlags
{
    DDSD_CAPS = 0x01,
    DDSD_HEIGHT = 0x02,
    DDSD_WIDTH = 0x04,
    DDSD_PITCH = 0x08,
    DDSD_PIXELFORMAT = 0x1000,
    DDSD_MIPMAPCOUNT = 0x20000,
    DDSD_LINEARSIZE = 0x80000,
    DDSD_DEPTH = 0x800000,
};

enum DDSPixelFormatFlags
{
    DDPF_ALPHAPIXELS = 0x01,
    DDPF_ALPHA = 0x02,
    DDPF_FOURCC = 0x04,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000,
};

enum DDSCaps
{
    DDSCAPS_COMPLEX = 0x08,
    DDSCAPS_TEXTURE = 0x1000,
    DDSCAPS_MIPMAP = 0x400000,
};

enum DDSCaps2
{
    DDSCAPS2_CUBEMAP = 0x200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    DDSCAPS2_VOLUME = 0x200000,
};

enum DDSImageFormat
{
    D3DFMT_UNKNOWN = 0,
    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_R3G3B2 = 27,
    //D3DFMT_A8           = 28,
    //D3DFMT_A8R3G3B2     = 29,
    D3DFMT_X4R4G4B4 = 30,
    //D3DFMT_A2B10G10R10  = 31,
    D3DFMT_A8B8G8R8 = 32,
    D3DFMT_X8B8G8R8 = 33,
    //D3DFMT_G16R16       = 34,
    //D3DFMT_A2R10G10B10  = 35,
    //D3DFMT_A16B16G16R16 = 36,

    D3DFMT_DXT1 = 0x31545844,
    D3DFMT_DXT2 = 0x32545844,
    D3DFMT_DXT3 = 0x33545844,
    D3DFMT_DXT4 = 0x34545844,
    D3DFMT_DXT5 = 0x35545844,
    D3DFMT_DX10 = 0x30315844,
};

constexpr uint32_t DDS_5TO8[] =
{
    0,   8,   16,  25,  33,  41,  49,  58,
    66,  74,  82,  90,  99,  107, 115, 123,
    132, 140, 148, 156, 165, 173, 181, 189,
    197, 206, 214, 222, 230, 239, 247, 255
};

constexpr uint32_t DDS_6TO8[] =
{
    0,   4,   8,   12,  16,  20,  24,  28,
    32,  36,  40,  45,  49,  53,  57,  61,
    65,  69,  73,  77,  81,  85,  89,  93,
    97,  101, 105, 109, 113, 117, 121, 125,
    130, 134, 138, 142, 146, 150, 154, 158,
    162, 166, 170, 174, 178, 182, 186, 190,
    194, 198, 202, 206, 210, 215, 219, 223,
    227, 231, 235, 239, 243, 247, 251, 255
};

class DDSImage
{
    public:
        DDSImage(const std::string& filename);

        bool HasFlag(DDSFlags flag) const { return (_DDSHeader._flags & flag) == flag; }
        bool HasFlag(DDSPixelFormatFlags flag) const { return (_DDSHeader._pixelFormat._flags & flag) == flag; }

        DDSImageFormat GetImageFormat() const { return _DDSHeader._imageFormat; }

        uint32_t GetWidth()  const { return _DDSHeader._width; }
        uint32_t GetHeight() const { return _DDSHeader._height; }
        uint32_t GetMipMapCount() const { return _DDSHeader._mipMapCount; }
        uint32_t GetBitDepth() const { return _DDSHeader._pixelFormat._RGBBitCount; }

    protected:
        DDSImage();

        void LoadDDSHeader(FileReader* reader, bool strict = true);
        void LoadDXT10Header(FileReader* reader);
        void LoadDDSImage(FileReader* reader);

        void DecompressDXT1(FileReader* reader);

        bool HasPixelBitMask(uint32_t r, uint32_t g, uint32_t b, uint32_t a) const;

        struct DDSHeader
        {
            DDSImageFormat _imageFormat;
            uint32_t _magic;
            uint32_t _size;
            uint32_t _flags;
            uint32_t _height;
            uint32_t _width;
            uint32_t _pitchOrLinearSize;
            uint32_t _depth;
            uint32_t _mipMapCount;
            uint32_t _reserved[11];

            struct DDSPixelFormat
            {
                uint32_t _size;
                uint32_t _flags;
                uint32_t _fourCC;
                uint32_t _RGBBitCount;
                uint32_t _RBitMask;
                uint32_t _GBitMask;
                uint32_t _BBitMask;
                uint32_t _ABitMask;
            }
            _pixelFormat;

            uint32_t _caps;
            uint32_t _caps2;
            uint32_t _caps3;
            uint32_t _caps4;
            uint32_t _reserved2;
        }
        _DDSHeader;

        struct DXT10Header
        {
            uint32_t _format;
            uint32_t _resourceDimension;
            uint32_t _miscFlags;
            uint32_t _arraySize;
            uint32_t _miscFlags2;
        }
        _DXT10Header;

        std::vector<uint32_t*> _imageData;
};

#endif//INC_GDCL_EXTRACTOR_DDS_IMAGE_H