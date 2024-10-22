#ifndef INC_GDCL_GAME_QUEST_H
#define INC_GDCL_GAME_QUEST_H

#include <vector>
#include <filesystem>
#include "FileData.h"
#include "FileReader.h"
#include "JSONObject.h"
#include "GDDataBlock.h"
#include "UID.h"

class Quest : public FileData, public JSONObject
{
    public:
        struct QuestTask : public JSONObject
        {
            QuestTask() {}
            QuestTask(EncodedFileReader* reader) { Read(reader); }

            web::json::value ToJSON() const;

            void Read(EncodedFileReader* reader);
            void Write(EncodedFileWriter* writer);

            uint32_t _id1;
            UID16    _id2;
            uint32_t _state;
            uint8_t  _isInProgress;
            uint8_t  _unk1;
            std::vector<uint32_t> _objectives;
        };

        struct QuestData : public JSONObject
        {
            QuestData() {}
            QuestData(EncodedFileReader* reader) { Read(reader); }

            web::json::value ToJSON() const;

            void Read(EncodedFileReader* reader);
            void Write(EncodedFileWriter* writer);

            uint32_t _id1;
            UID16    _id2;
            std::vector<QuestTask> _tasks;
        };

        Quest() {}
        Quest(const std::filesystem::path& path) { ReadFromFile(path); }
        Quest(uint8_t* data, size_t size) { ReadFromBuffer(data, size); }

        size_t GetBufferSize() const;

        web::json::value ToJSON() const;

        bool ReadFromFile(const std::filesystem::path& path);
        bool ReadFromBuffer(uint8_t* data, size_t size);
        bool WriteToFile(const std::filesystem::path& path);
        bool WriteToBuffer(uint8_t* data, size_t size);

        UID16 _id;

        // Tokens block, ID = 10, Version = 2
        struct QuestTokensBlock : public GDDataBlock
        {
            QuestTokensBlock() : GDDataBlock(0x0A, 0x02) {}

            web::json::value ToJSON() const;

            std::vector<std::string> _questTokens;
        }
        _tokensBlock;

        // Quest data block, ID = 11, Version = 3,4
        struct QuestDataBlock : public GDDataBlock
        {
            QuestDataBlock() : GDDataBlock(0x0B, 0x0C) {}

            web::json::value ToJSON() const;

            std::vector<QuestData> _questData;
        }
        _dataBlock;

    private:
        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);

        void ReadHeaderData(EncodedFileReader* reader);
        void ReadTokensBlock(EncodedFileReader* reader);
        void ReadDataBlock(EncodedFileReader* reader);

        void WriteHeaderData(EncodedFileWriter* writer);
        void WriteTokensBlock(EncodedFileWriter* writer);
        void WriteDataBlock(EncodedFileWriter* writer);
};


#endif//INC_GDCL_GAME_QUEST_H