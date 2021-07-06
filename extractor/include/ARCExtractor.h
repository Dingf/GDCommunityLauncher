#ifndef INC_GDCL_EXTRACTOR_ARC_EXTRACTOR_H
#define INC_GDCL_EXTRACTOR_ARC_EXTRACTOR_H

#include <filesystem>
#include <memory>
#include "FileReader.h"

// = "ARC\0"
constexpr uint32_t ARC_MAGIC = 0x00435241;

class ARCExtractor
{
    public:
        static bool Extract(const std::filesystem::path& src, const std::filesystem::path& outputDir);

    private:
        ARCExtractor(const std::filesystem::path& filename);

        void ReadARCHeader(FileReader* reader);
        void ReadARCFileChunks(FileReader* reader);
        void ExtractARCFiles(FileReader* reader, const std::filesystem::path& outputDir);

        struct ARCHeader
        {
            uint32_t _magic;
            uint32_t _version;
            uint32_t _fileCount;
            uint32_t _chunkCount;
            uint32_t _chunkSize;
            uint32_t _filenameSize;
            uint32_t _chunkStart;
        }
        _header;

        struct ARCFileChunk
        {
            ARCFileChunk() {}
            ARCFileChunk(FileReader* reader) { Read(reader); }

            void Read(FileReader* reader);

            uint32_t _offset;
            uint32_t _compressedSize;
            uint32_t _decompressedSize;
        };

        struct ARCFileInfo
        {
            ARCFileInfo() {}
            ARCFileInfo(FileReader* reader) { Read(reader); }

            void Read(FileReader* reader);

            void Decompress(const uint8_t* buffer, std::vector<ARCFileChunk>& chunks);

            void Write(const std::filesystem::path& outputDir);

            uint32_t _type; //TODO: Make this an enum?
            uint32_t _offset;
            uint32_t _compressedSize;
            uint32_t _decompressedSize;
            uint32_t _hash;
            uint64_t _fileTime;
            uint32_t _chunkCount;
            uint32_t _chunkStartIndex;
            uint32_t _stringLength;
            uint32_t _stringOffset;

            std::string _filename;
            std::unique_ptr<char[]> _fileData;
        };

        std::filesystem::path _arcName;
        std::vector<ARCFileChunk> _fileChunks;
};

#endif//INC_GDCL_EXTRACTOR_ARC_EXTRACTOR_H