#ifndef INC_GDCL_FILE_READER_H
#define INC_GDCL_FILE_READER_H

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include "FileBuffer.h"

class FileReader : public FileBuffer
{
    public:
        FileReader(const std::filesystem::path& filename);
        FileReader(uint8_t* buffer, size_t size);

        bool HasData() const { return (_bufferSize > 0); }

        virtual float ReadFloat();
        virtual uint8_t ReadInt8();
        virtual uint16_t ReadInt16();
        virtual uint32_t ReadInt32();
                uint64_t ReadInt64();
        virtual std::string ReadString();
        virtual std::wstring ReadWideString();
};

class EncodedFileReader : public FileReader
{
    public:
        EncodedFileReader(const std::filesystem::path& filename);
        EncodedFileReader(uint8_t* buffer, size_t size);

        float ReadFloat(bool update);
        float ReadFloat() { return ReadFloat(true); }

        uint8_t ReadInt8(bool update);
        uint8_t ReadInt8() { return ReadInt8(true); }

        uint16_t ReadInt16(bool update);
        uint16_t ReadInt16() { return ReadInt16(true); }

        uint32_t ReadInt32(bool update);
        uint32_t ReadInt32() { return ReadInt32(true); }

        std::string ReadString();
        std::wstring ReadWideString();

    private:
        void UpdateKey(uint32_t val);

        uint32_t _key;
        uint32_t _table[256];
};

#endif//INC_GDCL_FILE_READER_H