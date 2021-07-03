#include "DDSImage.h"
#include "FileReader.h"
#include "Log.h"

DDSImage::DDSImage()
{
    memset(&_DDSHeader, 0, sizeof(DDSHeader));
    memset(&_DXT10Header, 0, sizeof(DXT10Header));
}

DDSImage::DDSImage(const std::string& filename)
{
    memset(&_DDSHeader, 0, sizeof(DDSHeader));
    memset(&_DXT10Header, 0, sizeof(DXT10Header));

    std::shared_ptr<FileReader> reader = FileReader::Open(filename);
    if (reader == nullptr)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", filename));

    LoadDDSHeader(reader.get());
    LoadDDSImage(reader.get());
}

bool DDSImage::HasPixelBitMask(uint32_t r, uint32_t g, uint32_t b, uint32_t a) const
{
    return (_DDSHeader._pixelFormat._RBitMask == r) &&
           (_DDSHeader._pixelFormat._GBitMask == g) &&
           (_DDSHeader._pixelFormat._BBitMask == b) &&
           (_DDSHeader._pixelFormat._ABitMask == a);
}

void DDSImage::LoadDDSHeader(FileReader* reader, bool strict)
{
    _DDSHeader._magic = reader->ReadInt32();
    _DDSHeader._size = reader->ReadInt32();
    _DDSHeader._flags = reader->ReadInt32();
    _DDSHeader._height = reader->ReadInt32();
    _DDSHeader._width = reader->ReadInt32();
    _DDSHeader._pitchOrLinearSize = reader->ReadInt32();
    _DDSHeader._depth = reader->ReadInt32();
    _DDSHeader._mipMapCount = reader->ReadInt32();

    for (uint32_t i = 0; i < 11; ++i)
    {
        _DDSHeader._reserved[i] = reader->ReadInt32();
    }

    _DDSHeader._pixelFormat._size = reader->ReadInt32();
    _DDSHeader._pixelFormat._flags = reader->ReadInt32();
    _DDSHeader._pixelFormat._fourCC = reader->ReadInt32();
    _DDSHeader._pixelFormat._RGBBitCount = reader->ReadInt32();
    _DDSHeader._pixelFormat._RBitMask = reader->ReadInt32();
    _DDSHeader._pixelFormat._GBitMask = reader->ReadInt32();
    _DDSHeader._pixelFormat._BBitMask = reader->ReadInt32();
    _DDSHeader._pixelFormat._ABitMask = reader->ReadInt32();

    _DDSHeader._caps = reader->ReadInt32();
    _DDSHeader._caps2 = reader->ReadInt32();
    _DDSHeader._caps3 = reader->ReadInt32();
    _DDSHeader._caps4 = reader->ReadInt32();
    _DDSHeader._reserved2 = reader->ReadInt32();

    uint32_t expectedFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    uint32_t expectedCaps = DDSCAPS_TEXTURE;

    if (_DDSHeader._mipMapCount > 1)
    {
        expectedFlags |= DDSD_MIPMAPCOUNT;
        expectedCaps |= DDSCAPS_COMPLEX;
    }

    if (_DDSHeader._depth > 1)
    {
        expectedFlags |= DDSD_DEPTH;
        expectedCaps |= DDSCAPS_COMPLEX;
    }

    if (_DDSHeader._caps2 & DDSCAPS2_CUBEMAP)
    {
        expectedCaps |= DDSCAPS_COMPLEX;
    }

    if (HasFlag(DDPF_FOURCC) && (_DDSHeader._pitchOrLinearSize > 0))
    {
        expectedFlags |= DDSD_LINEARSIZE;
    }

    if ((HasFlag(DDPF_RGB) || HasFlag(DDPF_YUV) || HasFlag(DDPF_LUMINANCE) || HasFlag(DDPF_ALPHA)) && (_DDSHeader._pitchOrLinearSize > 0))
    {
        expectedFlags |= DDSD_PITCH;
    }

    if (((_DDSHeader._magic != DDSR_MAGIC) && (_DDSHeader._magic != DDS_MAGIC)) ||
        (_DDSHeader._size != 124) ||
        (_DDSHeader._pixelFormat._size != 32) ||
        (strict && ((_DDSHeader._flags & expectedFlags) == expectedFlags)) ||
        (strict && ((_DDSHeader._caps & expectedCaps) == expectedCaps)))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified file does not contain valid DDS image data"));

    // For non-strict cases (like with TEX images), a lot of the values aren't populated properly. In that case,
    // we fill the values in with what we expect the data to look like
    if (!strict)
    {
        _DDSHeader._flags |= expectedFlags;
        _DDSHeader._caps |= expectedCaps;
        _DDSHeader._pixelFormat._RBitMask = 0x00FF0000;
        _DDSHeader._pixelFormat._GBitMask = 0x0000FF00;
        _DDSHeader._pixelFormat._BBitMask = 0x000000FF;
        _DDSHeader._pixelFormat._ABitMask = (GetBitDepth() == 32) ? 0xFF000000 : 0x00000000;
    }

    if (HasFlag(DDPF_FOURCC))
    {
        _DDSHeader._imageFormat = static_cast<DDSImageFormat>(_DDSHeader._pixelFormat._fourCC);
        if (GetImageFormat() == D3DFMT_DX10)
            LoadDXT10Header(reader);
    }
    else if (HasFlag(DDPF_RGB))
    {
        uint32_t bitDepth = GetBitDepth();

        if (bitDepth == 8)
        {
            if (HasPixelBitMask(0xE0, 0x1C, 0x03, 0x00))
                _DDSHeader._imageFormat = D3DFMT_R3G3B2;
        }
        if (bitDepth == 16)
        {
            if (HasPixelBitMask(0x7C00, 0x03E0, 0x001F, 0x8000))
                _DDSHeader._imageFormat = D3DFMT_A1R5G5B5;
            else if (HasPixelBitMask(0x7C00, 0x03E0, 0x001F, 0x0000))
                _DDSHeader._imageFormat = D3DFMT_X1R5G5B5;
            else if (HasPixelBitMask(0x0F00, 0x00F0, 0x000F, 0xF000))
                _DDSHeader._imageFormat = D3DFMT_A4R4G4B4;
            else if (HasPixelBitMask(0x0F00, 0x00F0, 0x000F, 0x0000))
                _DDSHeader._imageFormat = D3DFMT_X4R4G4B4;
            else if (HasPixelBitMask(0xF800, 0x07E0, 0x001F, 0x0000))
                _DDSHeader._imageFormat = D3DFMT_R5G6B5;
        }
        else if (bitDepth == 24)
        {
            if (HasPixelBitMask(0xFF0000, 0x00FF00, 0x0000FF, 0x000000))
                _DDSHeader._imageFormat = D3DFMT_R8G8B8;
        }
        else if (bitDepth == 32)
        {
            if (HasPixelBitMask(0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000))
                _DDSHeader._imageFormat = D3DFMT_A8R8G8B8;
            else if (HasPixelBitMask(0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000))
                _DDSHeader._imageFormat = D3DFMT_X8R8G8B8;
            else if (HasPixelBitMask(0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000))
                _DDSHeader._imageFormat = D3DFMT_A8B8G8R8;
            else if (HasPixelBitMask(0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000))
                _DDSHeader._imageFormat = D3DFMT_X8B8G8R8;
        }
    }

    if (GetImageFormat() == D3DFMT_UNKNOWN)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified file contains an invalid or unsupported DDS image format"));
}

void DDSImage::LoadDXT10Header(FileReader* reader)
{
    _DXT10Header._format = reader->ReadInt32();
    _DXT10Header._resourceDimension = reader->ReadInt32();
    _DXT10Header._miscFlags = reader->ReadInt32();
    _DXT10Header._arraySize = reader->ReadInt32();
    _DXT10Header._miscFlags2 = reader->ReadInt32();
}

uint32_t GetDXT1Color(uint32_t c0, uint32_t c1, uint32_t a, uint32_t code)
{
    uint32_t r, g, b;
    switch (code)
    {
        case 0:
            r = DDS_5TO8[(c0 & 0xF800) >> 11];
            g = DDS_6TO8[(c0 & 0x07E0) >> 5];
            b = DDS_5TO8[c0 & 0x001F];
        case 1:
            r = DDS_5TO8[(c1 & 0xF800) >> 11];
            g = DDS_6TO8[(c1 & 0x07E0) >> 5];
            b = DDS_5TO8[c1 & 0x001F];
        case 2:
            if (c0 > c1)
            {
                r = ((DDS_5TO8[(c0 & 0xF800) >> 11] * 2) + DDS_5TO8[(c1 & 0xF800) >> 11]) / 3;
                g = ((DDS_6TO8[(c0 & 0x07E0) >> 5] * 2)  + DDS_6TO8[(c1 & 0x07E0) >> 5]) / 3;
                b = ((DDS_5TO8[c0 & 0x001F] * 2)         + DDS_5TO8[c1 & 0x001F]) / 3;
            }
            else
            {
                r = (DDS_5TO8[(c0 & 0xF800) >> 11] + DDS_5TO8[(c1 & 0xF800) >> 11]) / 2;
                g = (DDS_6TO8[(c0 & 0x07E0) >> 5]  + DDS_6TO8[(c1 & 0x07E0) >> 5]) / 2;
                b = (DDS_5TO8[c0 & 0x001F]         + DDS_5TO8[c1 & 0x001F]) / 2;
            }
        case 3:
            if (c0 > c1)
            {
                r = (DDS_5TO8[(c0 & 0xF800) >> 11] + (DDS_5TO8[(c1 & 0xF800) >> 11] * 2)) / 3;
                g = (DDS_6TO8[(c0 & 0x07E0) >> 5]  + (DDS_6TO8[(c1 & 0x07E0) >> 5] * 2)) / 3;
                b = (DDS_5TO8[c0 & 0x001F]         + (DDS_5TO8[c1 & 0x001F] * 2)) / 3;
            }
            else
                return 0;
    }
    return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

void DDSImage::DecompressDXT1(FileReader* reader)
{
    uint32_t width = GetWidth();
    uint32_t height = GetHeight();

    uint32_t w = (width + 3) / 4;
    uint32_t h = (height + 3) / 4;

    uint32_t* buffer = new uint32_t[width * height];
    for (uint32_t i = 0; i < h; ++i)
    {
        uint32_t colOffset = 4 * width * i;
        for (uint32_t j = 0; j < w; ++j)
        {
            uint32_t c0 = reader->ReadInt16();
            uint32_t c1 = reader->ReadInt16();
            uint32_t codes = reader->ReadInt32();

            uint32_t count = 0;
            uint32_t rowOffset = 4 * j;
            uint32_t index = colOffset + rowOffset;
            for (uint32_t k = 0; k < 16; ++k, ++index)
            {
                if (rowOffset + count < width)
                {
                    uint32_t code = (codes & (0x03 << (k * 2))) >> (k * 2);
                    buffer[index] = GetDXT1Color(c0, c1, 0xFF, code);
                }
                if ((++count) >= 4)
                {
                    count = 0;
                    index += width;
                    index -= 4;
                }
            }
        }
    }
    _imageData.push_back(buffer);
}

void DDSImage::LoadDDSImage(FileReader* reader)
{
    if (!_imageData.empty())
        return;

    uint32_t width = GetWidth();
    uint32_t height = GetHeight();

    DDSImageFormat format = GetImageFormat();

    if (format == D3DFMT_UNKNOWN)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Invalid or unsupported DDS image format"));

    uint32_t mipmaps = GetMipMapCount();

    //TODO: Fix me when you decide to add mipmap support
    //for (uint32_t i = 0; i < mipmaps; ++i)
    for (uint32_t i = 0; i < 1; ++i)
    {
        switch (format)
        {
            case D3DFMT_DXT1:
                DecompressDXT1(reader);
            /*case D3DFMT_DXT2:
                DecompressDXT2(reader);
            case D3DFMT_DXT3:
                DecompressDXT3(reader);
            case D3DFMT_DXT4:
                DecompressDXT4(reader);
            case D3DFMT_DXT5:
                DecompressDXT5(reader);*/
        }
    }
}