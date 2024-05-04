#ifndef INC_GDCL_FILE_WRITER_H
#define INC_GDCL_FILE_WRITER_H

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "FileBuffer.h"

class FileWriter : public FileBuffer
{
    public:
        FileWriter(size_t size);
        FileWriter(uint8_t* buffer, size_t size);

        virtual void BufferFloat(float val);
        virtual void BufferInt8(uint8_t val);
        virtual void BufferInt16(uint16_t val);
        virtual void BufferInt32(uint32_t val);
                void BufferInt64(uint64_t val);
        virtual void BufferString(const std::string& val);
        virtual void BufferWideString(const std::wstring& val);

        void CopyFromBuffer(uint8_t* buffer, size_t size);

        void WriteToFile(const std::filesystem::path& filename);
};

class EncodedFileWriter : public FileWriter
{
    public:
        EncodedFileWriter(size_t size);

        void BufferFloat(float val, bool update);
        void BufferFloat(float val) { BufferFloat(val, true); }

        void BufferInt8(uint8_t val, bool update);
        void BufferInt8(uint8_t val) { BufferInt8(val, true); }

        void BufferInt16(uint16_t val, bool update);
        void BufferInt16(uint16_t val) { BufferInt16(val, true); }

        void BufferInt32(uint32_t val, bool update);
        void BufferInt32(uint32_t val) { BufferInt32(val, true); }

        void BufferInt64(uint64_t val) = delete;
        void CopyFromBuffer(uint8_t* buffer, size_t size) = delete;

        void BufferString(std::string val);
        void BufferWideString(std::wstring val);

    private:
        void UpdateKey(uint32_t val);

        uint32_t _key;
        uint32_t _table[256];
};

#endif//INC_GDCL_FILE_WRITER_H