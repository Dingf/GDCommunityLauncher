#include "Log.h"
#include "FileReader.h"
#include "GDDataBlock.h"
#include "SharedStash.h"

bool SharedStash::ReadFromFile(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path))
    {
        EncodedFileReader reader(path);
        if (!reader.HasData())
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", path.string().c_str());
            return false;
        }

        try
        {
            ReadSharedStashData(&reader);
            return true;
        }
        catch (std::runtime_error&)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash file \"%\"", path.string().c_str());
            return false;
        }
    }
    return false;
}

void SharedStash::WriteToFile(const std::filesystem::path& path)
{
    std::filesystem::path writePath = path;
    if (std::filesystem::exists(path))
    {
        if (std::filesystem::is_regular_file(path))
        {
            writePath += "_tmp";
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Tried to write shared stash to path \"%\" which is not a file", path.string().c_str());
        }
    }


}

void SharedStash::ReadSharedStashData(EncodedFileReader* reader)
{
    uint32_t fileVersion = reader->ReadInt32();
    if (fileVersion != 2)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file version is invalid or unsupported"));

    _headerBlock.ReadBlockStart(reader);

    _headerBlock._unk1 = reader->ReadInt32(false);
    _headerBlock._stashModName = reader->ReadString();

    // Vanilla = 0x00, AoM = 0x01, FG = 0x02
    //   Since you can't install FG without AoM though, FG essentially has a value of 0x03
    _headerBlock._stashExpansions = 0;
    if (_headerBlock.GetBlockVersion() >= 5)
    {
        _headerBlock._stashExpansions = reader->ReadInt8();
    }

    uint32_t numTabs = reader->ReadInt32();
    for (uint32_t i = 0; i < numTabs; ++i)
    {
        ReadStashTab(reader);
    }

    _headerBlock.ReadBlockEnd(reader);
}

size_t SharedStash::CalculateBufferSize() const
{
    size_t size = 28;
    size += _headerBlock._stashModName.size();

    return size;
}

web::json::value SharedStash::SharedStashHeaderBlock::ToJSON()
{
    web::json::value obj = web::json::value::object();

    //TODO: Implement me

    return obj;
}