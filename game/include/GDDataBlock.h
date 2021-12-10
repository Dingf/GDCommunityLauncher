#ifndef INC_GDCL_GAME_GD_DATA_BLOCK_H
#define INC_GDCL_GAME_GD_DATA_BLOCK_H

#include "JSONObject.h"
#include "FileReader.h"
#include "FileWriter.h"

enum GDDataBlockFlags
{
    GD_DATA_BLOCK_FLAG_ID = 0x01,
    GD_DATA_BLOCK_FLAG_VERSION = 0x02
};

class GDDataBlock : public JSONObject
{
    public:
        GDDataBlock(uint32_t ID, uint32_t versionMask) : _blockID(ID), _blockVersionMask(versionMask) {}

        virtual web::json::value ToJSON();

        uint32_t GetBlockID()          const { return _blockID; }
        uint32_t GetBlockVersion()     const { return _blockVersion; }
        uint32_t GetBlockVersionMask() const { return _blockVersionMask; }
        uint32_t GetBlockLength()      const { return _blockLength; }

        void ReadBlockStart(EncodedFileReader* reader, uint32_t flags = (GD_DATA_BLOCK_FLAG_ID | GD_DATA_BLOCK_FLAG_VERSION));
        void ReadBlockEnd(EncodedFileReader* reader);

        void WriteBlockStart(EncodedFileWriter* writer, uint32_t flags = (GD_DATA_BLOCK_FLAG_ID | GD_DATA_BLOCK_FLAG_VERSION));
        void WriteBlockEnd(EncodedFileWriter* writer);

    private:
        const uint32_t _blockID;
        const uint32_t _blockVersionMask;

        int64_t  _blockStart;
        uint32_t _blockLength;
        uint32_t _blockVersion;
};

#endif//INC_GDCL_GAME_GD_DATA_BLOCK_H