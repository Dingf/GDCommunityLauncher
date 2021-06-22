#include "Quest.h"
#include "Log.h"

bool Quest::ReadFromFile(const std::filesystem::path& path)
{
    if (!IsValid() && std::filesystem::is_regular_file(path))
    {
        std::shared_ptr<EncodedFileReader> readerPtr = EncodedFileReader::Open(path.c_str());
        if (!readerPtr)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file: \"%\"", path.string().c_str());
            return false;
        }

        EncodedFileReader* reader = readerPtr.get();
        try
        {
            ReadHeaderData(reader);
            ReadTokensBlock(reader);
            ReadDataBlock(reader);

            SetState(true);
            return true;
        }
        catch (std::runtime_error&)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load quest file \"%\"", path.string().c_str());
            return false;
        }
    }
    return false;
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
    _tokensBlock.SetState(true);
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
    _dataBlock.SetState(true);
}

void Quest::QuestTask::Read(EncodedFileReader* reader)
{
    _id1 = reader->ReadInt32();
    _id2 = UID16(reader);

    GDDataBlock questTaskBlock(0x00, 0x00);
    questTaskBlock.ReadBlockStart(reader, GD_DATA_BLOCK_READ_ID);

    _state = reader->ReadInt32();
    _isInProgress = reader->ReadInt8();

    _objectives.clear();

    int32_t numObjects = ((int32_t)questTaskBlock.GetBlockLength() - 5) / 4;
    for (int32_t i = 0; i < numObjects; ++i)
    {
        _objectives.emplace_back(reader->ReadInt32());
    }

    questTaskBlock.ReadBlockEnd(reader);
    SetState(true);
}

void Quest::QuestData::Read(EncodedFileReader* reader)
{
    _id1 = reader->ReadInt32();
    _id2 = UID16(reader);

    GDDataBlock questDataBlock(0x00, 0x00);
    questDataBlock.ReadBlockStart(reader, GD_DATA_BLOCK_READ_ID);

    _tasks.clear();

    uint32_t numTasks = reader->ReadInt32();
    for (uint32_t i = 0; i < numTasks; ++i)
    {
        _tasks.emplace_back(reader);
    }

    questDataBlock.ReadBlockEnd(reader);
    SetState(true);
}