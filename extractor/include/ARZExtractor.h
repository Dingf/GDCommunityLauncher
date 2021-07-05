#ifndef INC_GDCL_EXTRACTOR_ARZ_EXTRACTOR_H
#define INC_GDCL_EXTRACTOR_ARZ_EXTRACTOR_H

#include <string>
#include <vector>
#include <filesystem>
#include "DBRecord.h"
#include "FileReader.h"

enum ARZDataType
{
    ARZ_DATA_TYPE_INT = 0,
    ARZ_DATA_TYPE_FLOAT = 1,
    ARZ_DATA_TYPE_STRING = 2,
    ARZ_DATA_TYPE_BOOL = 3,
};

class ARZExtractor
{
    public:
        static bool Extract(const std::filesystem::path& src, const std::filesystem::path& outputDir);

    private:
        ARZExtractor();

        void ReadARZHeader(FileReader* reader);
        void ReadARZStrings(FileReader* reader);
        void ExtractARZRecords(FileReader* reader, const std::filesystem::path& outputDir);

        struct ARZHeader
        {
            uint16_t _format;
            uint16_t _version;
            uint32_t _recordStart;
            uint32_t _recordSize;
            uint32_t _recordCount;
            uint32_t _stringStart;
            uint32_t _stringSize;
        }
        _header;

        struct ARZRecord : public DBRecord
        {
            public:
                ARZRecord() {}
                ARZRecord(FileReader* reader) { ReadCompressed(reader); }

                void ReadCompressed(FileReader* reader);

                void Decompress(const uint8_t* buffer, const std::vector<std::string>& strings);

                void Write(const std::filesystem::path& outputDir);

            private:
                uint32_t _filenameID;
                std::string _filename;
                std::string _recordName;
                std::string _templateName;
                uint32_t _offset;
                uint32_t _compressedSize;
                uint32_t _decompressedSize;
                uint64_t _data;
        };

        std::vector<std::string> _strings;
};

#endif//INC_GDCL_EXTRACTOR_ARZ_EXTRACTOR_H