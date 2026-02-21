#include "Quest.h"
#include "Log.h"

size_t Quest::GetBufferSize() const
{
    size_t size = 64;
    for (size_t i = 0; i < _tokensBlock._questTokens.size(); ++i)
    {
        size += 4;
        size += _tokensBlock._questTokens[i].size();
    }

    for (size_t i = 0; i < _dataBlock._questData.size(); ++i)
    {
        size += 36;
        for (size_t j = 0; j < _dataBlock._questData[i]._tasks.size(); ++j)
        {
            size += 38;
            for (size_t k = 0; k < _dataBlock._questData[i]._tasks[j]._objectives.size(); ++k)
            {
                size += 4;
            }
        }
    }

    return size;
}

bool Quest::ReadFromFile(const std::filesystem::path& path)
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
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load quest file \"%\": %", path.string().c_str(), ex.what());
            return false;
        }
    }
    return false;
}

bool Quest::ReadFromBuffer(uint8_t* data, size_t size)
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

bool Quest::WriteToFile(const std::filesystem::path& path)
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

bool Quest::WriteToBuffer(uint8_t* data, size_t size)
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

void Quest::Read(EncodedFileReader* reader)
{
    ReadHeaderData(reader);
    ReadTokensBlock(reader);
    ReadDataBlock(reader);
}

void Quest::ReadHeaderData(EncodedFileReader* reader)
{
    uint32_t signature = reader->ReadInt32();
    uint32_t fileVersion = reader->ReadInt32();

    if ((signature != 1481921361) || (fileVersion != 0))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file signature or version is invalid"));

    _id = UID16(reader);
}

void Quest::ReadTokensBlock(EncodedFileReader* reader)
{
    _tokensBlock.ReadBlockStart(reader);

    _tokensBlock._questTokens.clear();

    uint32_t numTokens = reader->ReadInt32();
    for (uint32_t i = 0; i < numTokens; ++i)
    {
        _tokensBlock._questTokens.push_back(reader->ReadString());
    }

    _tokensBlock.ReadBlockEnd(reader);
}

void Quest::ReadDataBlock(EncodedFileReader* reader)
{
    _dataBlock.ReadBlockStart(reader);

    _dataBlock._questData.clear();

    uint32_t numQuests = reader->ReadInt32();
    for (uint32_t i = 0; i < numQuests; ++i)
    {
        _dataBlock._questData.emplace_back(reader);
    }

    _dataBlock.ReadBlockEnd(reader);
}

void Quest::QuestTask::Read(EncodedFileReader* reader)
{
    _id1 = reader->ReadInt32();
    _id2 = UID16(reader);

    GDDataBlock questTaskBlock(0x00, 0x00);
    questTaskBlock.ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_ID);

    _state = reader->ReadInt32();
    _isInProgress = reader->ReadInt8();

    _objectives.clear();

    int32_t numObjects = ((int32_t)questTaskBlock.GetBlockLength() - 5) / 4;

    for (int32_t i = 0; i < numObjects; ++i)
    {
        _objectives.emplace_back(reader->ReadInt32());
    }

    // Added in 1.2.1, some sort of boolean value?
    _unk1 = reader->ReadInt8();

    questTaskBlock.ReadBlockEnd(reader);
}

void Quest::QuestTask::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32(_id1);
    _id2.WriteUID16(writer);

    GDDataBlock questTaskBlock(0x00, 0x00);
    questTaskBlock.WriteBlockStart(writer, GD_DATA_BLOCK_FLAG_ID);

    writer->BufferInt32(_state);
    writer->BufferInt8(_isInProgress);

    for (uint32_t i = 0; i < _objectives.size(); ++i)
    {
        writer->BufferInt32(_objectives[i]);
    }

    writer->BufferInt8(_unk1);

    questTaskBlock.WriteBlockEnd(writer);
}

void Quest::QuestData::Read(EncodedFileReader* reader)
{
    _id1 = reader->ReadInt32();
    _id2 = UID16(reader);

    GDDataBlock questDataBlock(0x00, 0x00);
    questDataBlock.ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_ID);

    _tasks.clear();

    uint32_t numTasks = reader->ReadInt32();
    for (uint32_t i = 0; i < numTasks; ++i)
    {
        _tasks.emplace_back(reader);
    }

    questDataBlock.ReadBlockEnd(reader);
}

void Quest::QuestData::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32(_id1);
    _id2.WriteUID16(writer);

    GDDataBlock questDataBlock(0x00, 0x00);
    questDataBlock.WriteBlockStart(writer, GD_DATA_BLOCK_FLAG_ID);

    uint32_t numTasks = (uint32_t)_tasks.size();
    writer->BufferInt32(numTasks);
    for (uint32_t i = 0; i < numTasks; ++i)
    {
        _tasks[i].Write(writer);
    }

    questDataBlock.WriteBlockEnd(writer);
}

void Quest::Write(EncodedFileWriter* writer)
{
    WriteHeaderData(writer);
    WriteTokensBlock(writer);
    WriteDataBlock(writer);
}

void Quest::WriteHeaderData(EncodedFileWriter* writer)
{
    writer->BufferInt32(1481921361);
    writer->BufferInt32(0);
    _id.WriteUID16(writer);
}

void Quest::WriteTokensBlock(EncodedFileWriter* writer)
{
    _tokensBlock.WriteBlockStart(writer);

    uint32_t numTokens = (uint32_t)_tokensBlock._questTokens.size();
    writer->BufferInt32(numTokens);
    for (uint32_t i = 0; i < numTokens; ++i)
    {
        writer->BufferString(_tokensBlock._questTokens[i]);
    }

    _tokensBlock.WriteBlockEnd(writer);
}

void Quest::WriteDataBlock(EncodedFileWriter* writer)
{
    _dataBlock.WriteBlockStart(writer);

    uint32_t numQuests = (uint32_t)_dataBlock._questData.size();
    writer->BufferInt32(numQuests);
    for (uint32_t i = 0; i < numQuests; ++i)
    {
        _dataBlock._questData[i].Write(writer);
    }

    _dataBlock.WriteBlockEnd(writer);
}

void to_json(json& j, const Quest::QuestTask& data)
{
    j = json
    {
        { "ID1",        data._id1 },
        { "ID2",        data._id2 },
        { "State",      data._state },
        { "InProgress", data._isInProgress },
        { "Unknown1",   data._unk1 },
        { "Objectives", data._objectives },
    };
}

void from_json(const json& j, Quest::QuestTask& data)
{
    j.at("ID1")       .get_to(data._id1);
    j.at("ID2")       .get_to(data._id2);
    j.at("State")     .get_to(data._state);
    j.at("InProgress").get_to(data._isInProgress);
    j.at("Unknown1")  .get_to(data._unk1);
    j.at("Objectives").get_to(data._objectives);
}

void to_json(json& j, const Quest::QuestData& data)
{
    j = json
    {
        { "ID1",   data._id1 },
        { "ID2",   data._id2 },
        { "Tasks", data._tasks },
    };
}

void from_json(const json& j, Quest::QuestData& data)
{
    j.at("ID1")  .get_to(data._id1);
    j.at("ID2")  .get_to(data._id2);
    j.at("Tasks").get_to(data._tasks);
}

void to_json(json& j, const Quest& data)
{
    j = json
    {
        { "ID",          data._id },
        { "TokensBlock", data._tokensBlock },
        { "DataBlock",   data._dataBlock },
    };
}

void from_json(const json& j, Quest& data)
{
    j.at("ID")         .get_to(data._id);
    j.at("TokensBlock").get_to(data._tokensBlock);
    j.at("DataBlock")  .get_to(data._dataBlock);
}

void to_json(json& j, const Quest::QuestTokensBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Tokens",       data._questTokens },
    };
}

void from_json(const json& j, Quest::QuestTokensBlock& data)
{
    j.at("Tokens").get_to(data._questTokens);
}

void to_json(json& j, const Quest::QuestDataBlock& data)
{
    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "Data",         data._questData },
    };
}

void from_json(const json& j, Quest::QuestDataBlock& data)
{
    j.at("Data").get_to(data._questData);
}