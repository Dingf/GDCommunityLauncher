#include <lz4.h>
#include "ARCExtractor.h"
#include "FileReader.h"
#include "Log.h"

ARCExtractor::ARCExtractor(const std::filesystem::path& filename)
{
    memset(&_header, 0, sizeof(ARCHeader));
    _arcName = filename.stem();
}

void ARCExtractor::Extract(const std::filesystem::path& src, const std::filesystem::path& outputDir)
{
    FileReader reader(src);
    if (!reader.HasData())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file \"%\" for reading", src.string().c_str()));

    ARCExtractor extractor(src);
    extractor.ReadARCHeader(&reader);
    extractor.ReadARCFileChunks(&reader);
    extractor.ExtractARCFiles(&reader, outputDir);
}

void ARCExtractor::ReadARCHeader(FileReader* reader)
{
    _header._magic = reader->ReadInt32();
    _header._version = reader->ReadInt32();

    if ((_header._magic != ARC_MAGIC) || (_header._version != 3))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Unsupported ARC format or version"));

    _header._fileCount = reader->ReadInt32();
    _header._chunkCount = reader->ReadInt32();
    _header._chunkSize = reader->ReadInt32();
    _header._filenameSize = reader->ReadInt32();
    _header._chunkStart = reader->ReadInt32();
}

void ARCExtractor::ReadARCFileChunks(FileReader* reader)
{
    reader->SetPosition(_header._chunkStart);
    for (uint32_t i = 0; i < _header._chunkCount; ++i)
    {
        _fileChunks.emplace_back(reader);
    }
}

void ARCExtractor::ExtractARCFiles(FileReader* reader, const std::filesystem::path& outputDir)
{
    const char* buffer = (const char*)reader->GetBuffer();
    uint32_t bufferSize = (uint32_t)reader->GetBufferSize();

    uint32_t filenameStart = _header._chunkStart + _header._chunkSize;
    uint32_t filenameIndex = filenameStart;

    reader->SetPosition(filenameStart + _header._filenameSize);
    for (uint32_t i = 0; i < _header._fileCount; ++i)
    {
        while ((filenameIndex < bufferSize) && (buffer[filenameIndex] != '\0'))
            filenameIndex++;

        ARCFileInfo info(reader);
        info._filename = std::string(&buffer[filenameStart], filenameIndex - filenameStart);
        filenameIndex++;
        filenameStart = filenameIndex;

        info.Decompress(reader->GetBuffer(), _fileChunks);
        info.Write(outputDir / _arcName);
    }
}

void ARCExtractor::ARCFileChunk::Read(FileReader* reader)
{
    _offset = reader->ReadInt32();
    _compressedSize = reader->ReadInt32();
    _decompressedSize = reader->ReadInt32();
}

void ARCExtractor::ARCFileInfo::Read(FileReader* reader)
{
    _type = reader->ReadInt32();
    _offset = reader->ReadInt32();
    _compressedSize = reader->ReadInt32();
    _decompressedSize = reader->ReadInt32();
    _hash = reader->ReadInt32();
    _fileTime = reader->ReadInt64();
    _chunkCount = reader->ReadInt32();
    _chunkStartIndex = reader->ReadInt32();
    _stringLength = reader->ReadInt32();
    _stringOffset = reader->ReadInt32();
}

void ARCExtractor::ARCFileInfo::Decompress(const uint8_t* buffer, std::vector<ARCFileChunk>& chunks)
{
    std::unique_ptr<char[]> decompressedBuffer(new char[_decompressedSize]);
    if ((_type == 1) && (_compressedSize == _decompressedSize))
    {
        memcpy(&decompressedBuffer[0], &buffer[_offset], _decompressedSize);
    }
    else
    {
        uint32_t fileIndex = 0;
        for (uint32_t i = 0; i < _chunkCount; ++i)
        {
            ARCFileChunk& chunk = chunks[_chunkStartIndex + i];
            if (chunk._compressedSize == chunk._decompressedSize)
            {
                memcpy(&decompressedBuffer[fileIndex], &buffer[chunk._offset], chunk._decompressedSize);
            }
            else
            {
                char* compressedBuffer = new char[chunk._compressedSize];
                memcpy(compressedBuffer, &buffer[chunk._offset], chunk._compressedSize);

                LZ4_decompress_safe(compressedBuffer, &decompressedBuffer[fileIndex], chunk._compressedSize, chunk._decompressedSize);
                delete[] compressedBuffer;
            }
            fileIndex += chunk._decompressedSize;
        }
    }
    _fileData = std::move(decompressedBuffer);
}

void ARCExtractor::ARCFileInfo::Write(const std::filesystem::path& outputDir)
{
    std::filesystem::path outputPath = outputDir / _filename;
    std::filesystem::path parentPath = outputPath.parent_path();
    if (!std::filesystem::is_directory(parentPath) && !std::filesystem::create_directories(parentPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not create directory path to %", parentPath.string().c_str()));

    std::ofstream out(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!out.is_open())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open record file % for writing", outputPath.string().c_str()));

    out.write(&_fileData[0], _decompressedSize);
    out.close();
}