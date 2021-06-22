#ifndef INC_GDCL_GAME_QUEST_H
#define INC_GDCL_GAME_QUEST_H

#include <vector>
#include <filesystem>
#include "FileReader.h"
#include "Object.h"
#include "GDDataBlock.h"
#include "UID.h"

class Quest : public Object
{
    public:
        struct QuestTask : public Object
        {
            QuestTask() {}
            QuestTask(EncodedFileReader* reader) { Read(reader); }

            void Read(EncodedFileReader* reader);

            uint32_t _id1;
            UID16    _id2;
            uint32_t _state;
            uint8_t  _isInProgress;
            std::vector<uint32_t> _objectives;
        };

        struct QuestData : public Object
        {
            QuestData() {}
            QuestData(EncodedFileReader* reader) { Read(reader); }

            void Read(EncodedFileReader* reader);

            uint32_t _id1;
            UID16    _id2;
            std::vector<QuestTask> _tasks;
        };

        Quest() {}
        Quest(const std::filesystem::path& path) { ReadFromFile(path); }

        bool ReadFromFile(const std::filesystem::path& path);

    private:
        void ReadHeaderData(EncodedFileReader* reader);
        void ReadTokensBlock(EncodedFileReader* reader);
        void ReadDataBlock(EncodedFileReader* reader);

        UID16 _id;

        // Tokens block, ID = 10, Version = 2
        struct QuestTokensBlock : public GDDataBlock
        {
            QuestTokensBlock() : GDDataBlock(0x0A, 0x02) {}

            std::vector<std::string> _questTokens;
        }
        _tokensBlock;

        // Quest data block, ID = 11, Version = 3
        struct QuestDataBlock : public GDDataBlock
        {
            QuestDataBlock() : GDDataBlock(0x0B, 0x04) {}

            std::vector<QuestData> _questData;
        }
        _dataBlock;
};


#endif//INC_GDCL_GAME_QUEST_H