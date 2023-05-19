#ifndef INC_GDCL_FILE_DATA_H
#define INC_GDCL_FILE_DATA_H

#include <string>
#include <chrono>
#include "FileReader.h"
#include "FileWriter.h"
#include "MD5.h"

class FileData
{
    public:
        virtual void Read(EncodedFileReader* reader) = 0;
        virtual void Write(EncodedFileWriter* writer) = 0;
        virtual size_t GetBufferSize() const = 0;
};

struct FileMetadata
{
    FileMetadata() {}
    FileMetadata(const std::filesystem::path& path) { Load(path); }

    bool operator==(const FileMetadata& rhs)
    {
        return ((_checksum == rhs._checksum) && (_modifiedTime == rhs._modifiedTime));
    }

    bool operator!=(const FileMetadata& rhs)
    {
        return !(*this == rhs);
    }

    bool IsEmpty() const { return (_checksum.empty() && (_modifiedTime == 0)); }

    void Load(const std::filesystem::path& path)
    {
        if (std::filesystem::is_regular_file(path))
        {
            _checksum = GenerateFileMD5(path);
            _modifiedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(path).time_since_epoch()).count();
        }
    }

    void Clear()
    {
        _checksum.clear();
        _modifiedTime = 0;
    }

    std::string _checksum;
    uint64_t _modifiedTime;
};

#endif//INC_GDCL_BUFFER_DATA_H