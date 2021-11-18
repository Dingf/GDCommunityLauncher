#include <set>
#include <iomanip>
#include <lz4.h>
#include "ARZExtractor.h"
#include "Log.h"

ARZExtractor::ARZExtractor()
{
    memset(&_header, 0, sizeof(ARZHeader));
}

void ARZExtractor::Extract(const std::filesystem::path& src, const std::filesystem::path& outputDir)
{
    FileReader reader(src);
    if (!reader.HasData())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to open file \"%\" for reading", src.string().c_str()));

    ARZExtractor extractor;
    extractor.ReadARZHeader(&reader);
    extractor.ReadARZStrings(&reader);
    extractor.ExtractARZRecords(&reader, outputDir);
}

void ARZExtractor::ReadARZHeader(FileReader* reader)
{
    _header._format = reader->ReadInt16();
    _header._version = reader->ReadInt16();

    if ((_header._format != 2) || (_header._version != 3))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Unsupported ARZ format or version"));

    _header._recordStart = reader->ReadInt32();
    _header._recordSize = reader->ReadInt32();
    _header._recordCount = reader->ReadInt32();
    _header._stringStart = reader->ReadInt32();
    _header._stringSize = reader->ReadInt32();
}

void ARZExtractor::ReadARZStrings(FileReader* reader)
{
    reader->SetPosition(_header._stringStart);

    uint32_t count = reader->ReadInt32();
    for (uint32_t i = 0; i < count; ++i)
    {
        _strings.push_back(reader->ReadString());
    }
}

void ARZExtractor::ExtractARZRecords(FileReader* reader, const std::filesystem::path& outputDir)
{
    reader->SetPosition(_header._recordStart);

    for (uint32_t i = 0; i < _header._recordCount; ++i)
    {
        ARZRecord record(reader);
        record.Decompress(reader->GetBuffer(), _strings);
        record.Write(outputDir);
    }
}

void ARZExtractor::ARZRecord::Decompress(const uint8_t* buffer, const std::vector<std::string>& strings)
{
    uint32_t filenameID = _filenameID;
    if (filenameID >= strings.size())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Tried to load string % which is outside the bounds of the array", filenameID));

    _filename = strings[filenameID];

    std::unique_ptr<char[]> compressedBuffer(new char[_compressedSize]);
    std::unique_ptr<char[]> decompressedBuffer(new char[_decompressedSize]);

    memcpy(&compressedBuffer[0], &buffer[_offset + 24], _compressedSize);

    LZ4_decompress_safe(&compressedBuffer[0], &decompressedBuffer[0], _compressedSize, _decompressedSize);

    uint32_t index = 0;
    while (index < _decompressedSize)
    {
        uint16_t type = *(uint16_t*)(&decompressedBuffer[index]);
        index += 2;
        uint16_t count = *(uint16_t*)(&decompressedBuffer[index]);
        index += 2;
        uint32_t keyID = *(uint32_t*)(&decompressedBuffer[index]);
        index += 4;

        if (keyID >= strings.size())
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Tried to load string % which is outside the bounds of the array", keyID));

        std::string key = strings[keyID];
        for (uint32_t j = 0; j < count; ++j)
        {
            switch (type)
            {
                case ARZ_DATA_TYPE_INT:
                {
                    int32_t val = *(int32_t*)(&decompressedBuffer[index]);
                    _variables[key].emplace_back(std::make_unique<Value>(val));
                    break;
                }
                case ARZ_DATA_TYPE_FLOAT:
                {
                    float val = *(float*)(&decompressedBuffer[index]);
                    _variables[key].push_back(std::make_unique<Value>(val));
                    break;
                }
                case ARZ_DATA_TYPE_STRING:
                {
                    uint32_t stringID = *(uint32_t*)(&decompressedBuffer[index]);
                    if (stringID >= strings.size())
                        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Tried to load string % which is outside the bounds of the array", stringID));

                    const std::string& val = strings[stringID];
                    if (key == "templateName")
                        _templateName = val;
                    else
                        _variables[key].emplace_back(std::make_unique<Value>(val.c_str()));
                    break;
                }
                case ARZ_DATA_TYPE_BOOL:
                {
                    int32_t val = *(int32_t*)(&decompressedBuffer[index]);
                    _variables[key].push_back(std::make_unique<Value>((bool)(val != 0)));
                    break;
                }
                default:
                {
                    throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Unknown ARZ data type = %", type));
                    break;
                }
            }
            index += 4;
        }
    }
}

void ARZExtractor::ARZRecord::ReadCompressed(FileReader* reader)
{
    _filenameID = reader->ReadInt32();
    _recordName = reader->ReadString();
    _offset = reader->ReadInt32();
    _compressedSize = reader->ReadInt32();
    _decompressedSize = reader->ReadInt32();
    _data = reader->ReadInt64();
}

void ARZExtractor::ARZRecord::Write(const std::filesystem::path& outputDir)
{
    std::filesystem::path outputPath = outputDir / _filename;
    std::filesystem::path parentPath = outputPath.parent_path();
    if (!std::filesystem::is_directory(parentPath) && !std::filesystem::create_directories(parentPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not create directory path to %", parentPath.string().c_str()));

    std::ofstream out(outputPath, std::ofstream::out);
    if (!out.is_open())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open record file % for writing", outputPath.string().c_str()));

    // Template name is a special case and appears before the other variables
    if (!_templateName.empty())
        out << "templateName," << _templateName << ",\n";

    for (auto it = _variables.begin(); it != _variables.end(); ++it)
    {
        out << it->first << ",";
        for (int i = 0; i < it->second.size(); ++i)
        {
            switch (it->second[i]->GetType())
            {
                case VALUE_TYPE_INT:
                    out << it->second[i]->ToInt();
                    break;
                case VALUE_TYPE_FLOAT:
                    out << std::fixed << std::setprecision(6) << it->second[i]->ToFloat();
                    break;
                case VALUE_TYPE_BOOL:
                    out << (int)it->second[i]->ToBool();
                    break;
                case VALUE_TYPE_STRING:
                    out << it->second[i]->ToString();
                    break;
            }

            if (i < it->second.size() - 1)
                out << ";";
        }
        out << ",\n";
    }
    out.close();
}