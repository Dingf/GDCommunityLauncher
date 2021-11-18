#include "TEXImage.h"
#include "Log.h"

bool TEXImage::Load(const std::filesystem::path& path)
{
    memset(&_TEXHeader, 0, sizeof(TEXHeader));

    FileReader reader(path);
    if (!reader.HasData())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", path.string().c_str()));

    try
    {
        LoadTEXHeader(&reader);
        LoadDDSHeader(&reader, false);
        LoadDDSImage(&reader);
    }
    catch (std::runtime_error&)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load TEX image \"%\"", path.string().c_str());
        return false;
    }
    return true;
}

void TEXImage::LoadTEXHeader(FileReader* reader)
{
    uint32_t buffer = reader->ReadInt32();
    _TEXHeader._magic = buffer & 0x00FFFFFF;
    _TEXHeader._version = (buffer & 0xFF000000) >> 24;

    if ((_TEXHeader._magic != TEX_MAGIC) || (_TEXHeader._version != 2))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified file is not a valid TEX image"));

    _TEXHeader._unk1 = reader->ReadInt32();
    _TEXHeader._size = reader->ReadInt32();
}