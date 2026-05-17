#include "FileReader.h"
#include "FileWriter.h"
#include "GDDataBlock.h"
#include "SharedStash.h"
#include "Log.h"

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

bool SharedStash::ReadFromBuffer(const uint8_t* data, size_t size)
{
    try
    {
        EncodedFileReader reader(data, size);
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
    try
    {
        EncodedFileWriter writer(GetBufferSize());
        Write(&writer);
        writer.WriteToFile(path);
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write to shared stash file \"%\": %", path.string().c_str(), ex.what());
        return false;
    }
    return true;
}

bool SharedStash::WriteToBuffer(uint8_t* data, size_t size)
{
    try
    {
        EncodedFileWriter writer(GetBufferSize());
        Write(&writer);
        memcpy(data, writer.GetBuffer(), std::min(size, GetBufferSize()));
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write shared stash data: %", ex.what());
        return false;
    }
    return true;
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
    ReadStashTabs(reader, _headerBlock.GetBlockVersion(), numTabs);

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
    WriteStashTabs(writer, _headerBlock.GetBlockVersion());

    _headerBlock.WriteBlockEnd(writer);
}

void to_json(json& j, const SharedStash& data)
{
    to_json(j, (const Stash&)data);
    j["HeaderBlock"] = data._headerBlock;
}

void from_json(const json& j, SharedStash& data)
{
    from_json(j, (Stash&)data);
    j.at("HeaderBlock").get_to(data._headerBlock);
}

void to_json(json& j, const SharedStash::SharedStashHeaderBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "ModName",      data._stashModName },
        { "Expansion",    data._stashExpansions },
        { "Unknown1",     data._unk1 },
    };
}

void from_json(const json& j, SharedStash::SharedStashHeaderBlock& data)
{
    j.at("ModName")  .get_to(data._stashModName);
    j.at("Expansion").get_to(data._stashExpansions);
    j.at("Unknown1") .get_to(data._unk1);
}