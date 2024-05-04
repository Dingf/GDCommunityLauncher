#include "GDDataBlock.h"
#include "Log.h"

void GDDataBlock::ReadBlockStart(EncodedFileReader* reader, uint32_t flags)
{
    if (flags & GD_DATA_BLOCK_FLAG_ID)
    {
        int32_t blockID = reader->ReadInt32();
        if (blockID != _blockID)
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The block ID read (%) does not match the expected block ID (%)", blockID, _blockID));
    }

    _blockLength = reader->ReadInt32(false);
    _blockStart = reader->GetPosition();

    if (flags & GD_DATA_BLOCK_FLAG_VERSION)
    {
        _blockVersion = reader->ReadInt32();
        if ((_blockVersionMask & (1 << (_blockVersion - 1))) == 0)
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The block version read for block % (%) is not currently supported.", _blockID, _blockVersion));
    }
}

void GDDataBlock::ReadBlockEnd(EncodedFileReader* reader)
{
    int64_t current = reader->GetPosition();
    if ((_blockStart + _blockLength != reader->GetPosition()) || (reader->ReadInt32(false) != 0))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read block end for block % (% != %)", _blockID, _blockStart + _blockLength, current));
}

web::json::value GDDataBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    obj[U("BlockID")] = GetBlockID();
    obj[U("BlockVersion")] = GetBlockVersion();

    return obj;
}

void GDDataBlock::WriteBlockStart(EncodedFileWriter* writer, uint32_t flags)
{
    if (flags & GD_DATA_BLOCK_FLAG_ID)
        writer->BufferInt32(_blockID);

    writer->BufferInt32(_blockLength, false);
    _blockStart = writer->GetPosition();

    if (flags & GD_DATA_BLOCK_FLAG_VERSION)
        writer->BufferInt32(_blockVersion);
}

void GDDataBlock::WriteBlockEnd(EncodedFileWriter* writer)
{
    writer->BufferInt32(0, false);
    int64_t current = writer->GetPosition();

    writer->SetPosition(_blockStart - 4);
    writer->BufferInt32(current - _blockStart - 4);
    writer->SetPosition(current);
}