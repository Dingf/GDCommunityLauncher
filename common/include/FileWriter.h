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

        virtual void WriteFloat(float val);
        virtual void WriteInt8(uint8_t val);
        virtual void WriteInt16(uint16_t val);
        virtual void WriteInt32(uint32_t val);
                void WriteInt64(uint64_t val);
        virtual void WriteString(const std::string& val);
        virtual void WriteWideString(const std::wstring& val);
};

class EncodedFileWriter : public FileWriter
{
    public:
        EncodedFileWriter(const std::filesystem::path& filename);

        void WriteFloat(float val, bool update);
        void WriteFloat(float val) { WriteFloat(val, true); }

        void WriteInt8(uint8_t val, bool update);
        void WriteInt8(uint8_t val) { WriteInt8(val, true); }

        void WriteInt16(uint16_t val, bool update);
        void WriteInt16(uint16_t val) { WriteInt16(val, true); }

        void WriteInt32(uint32_t val, bool update);
        void WriteInt32(uint32_t val) { WriteInt32(val, true); }

        void WriteString(std::string val);
        void WriteWideString(std::wstring val);

    private:
        void UpdateKey(uint32_t val);

        uint32_t _key;
        uint32_t _table[256];
};

#endif//INC_GDCL_FILE_WRITER_H