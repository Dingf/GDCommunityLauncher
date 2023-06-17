#include "Log.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "GDDataBlock.h"
#include "SharedStash.h"

size_t SharedStash::GetBufferSize() const
{
    size_t size = 28 + _headerBlock._stashModName.size();

    if (_headerBlock.GetBlockVersion() >= 5)
        size++;

    size += Stash::GetBufferSize();

    return size;
}

bool SharedStash::ReadFromFile(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path))
    {
        try
        {
            EncodedFileReader reader(path);
            if (!reader.HasData())
                throw std::runtime_error(std::string("Could not open file: \"") + path.string().c_str() + "\" for reading");

            Read(&reader);
            return true;
        }
        catch (std::runtime_error& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash file \"%\": %", path.string().c_str(), ex.what());
            return false;
        }
    }
    return false;
}

bool SharedStash::ReadFromBytes(std::vector<uint8_t>& data)
{
    try
    {
        EncodedFileReader reader(&data[0], data.size());
        Read(&reader);
        return true;
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load shared stash data: %", ex.what());
        return false;
    }
}

bool SharedStash::WriteToFile(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path))
    {
        std::filesystem::path tempPath = path;
        tempPath += "_tmp";

        try
        {
            EncodedFileWriter writer(GetBufferSize());
            Write(&writer);

            writer.WriteToFile(tempPath);

            std::filesystem::rename(tempPath, path);
        }
        catch (std::runtime_error&)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write to shared stash file \"%\"", path.string().c_str());
            return false;
        }

        return true;
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Tried to write shared stash to path \"%\" which is not a file", path.string().c_str());
    }
    return false;
}

void SharedStash::Read(EncodedFileReader* reader)
{
    uint32_t fileVersion = reader->ReadInt32(true);
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
    ReadStashTabs(reader, numTabs);

    _headerBlock.ReadBlockEnd(reader);
}

void SharedStash::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32(2);

    _headerBlock.WriteBlockStart(writer);

    writer->BufferInt32(_headerBlock._unk1, false);
    writer->BufferString(_headerBlock._stashModName);

    if (_headerBlock.GetBlockVersion() >= 5)
    {
        writer->BufferInt8(_headerBlock._stashExpansions);
    }

    writer->BufferInt32((uint32_t)_stashTabs.size());
    WriteStashTabs(writer);

    _headerBlock.WriteBlockEnd(writer);
}

web::json::value SharedStash::SharedStashHeaderBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();
    obj[U("ModName")] = JSONString(_stashModName);
    obj[U("Expansion")] = _stashExpansions;
    obj[U("Unknown1")] = _unk1;

    return obj;
}