#ifndef INC_GDCL_FILE_READER_H
#define INC_GDCL_FILE_READER_H

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>

class FileReader
{
    public:
        virtual ~FileReader();

        static std::shared_ptr<FileReader> Open(const std::filesystem::path& filename);

        int64_t GetPosition() const { return _bufferPos; }
        int64_t GetBufferSize() const { return _bufferSize; }

        void SetPosition(int64_t position) { _bufferPos = position; }

        const uint8_t* GetBuffer() { return _buffer; }

        virtual float ReadFloat();
        virtual uint8_t ReadInt8();
        virtual uint16_t ReadInt16();
        virtual uint32_t ReadInt32();
        virtual uint64_t ReadInt64();
        virtual std::string ReadString();
        virtual std::wstring ReadWideString();

    protected:
        FileReader(std::ifstream& in);

        uint8_t* _buffer;
        int64_t _bufferSize;
        int64_t _bufferPos;
};

class EncodedFileReader : public FileReader
{
    public:
        static std::shared_ptr<EncodedFileReader> Open(const std::filesystem::path& filename);

        float ReadFloat(bool update);
        float ReadFloat() { return ReadFloat(true); }

        uint8_t ReadInt8(bool update);
        uint8_t ReadInt8() { return ReadInt8(true); }

        uint16_t ReadInt16(bool update);
        uint16_t ReadInt16() { return ReadInt16(true); }

        uint32_t ReadInt32(bool update);
        uint32_t ReadInt32() { return ReadInt32(true); }

        uint64_t ReadInt64(bool update);
        uint64_t ReadInt64() { return ReadInt64(true); }

        std::string ReadString();
        std::wstring ReadWideString();

    private:
        EncodedFileReader(std::ifstream& in);

        void UpdateKey(uint32_t val);

        uint32_t _key;
        uint32_t _table[256];
};



#endif//INC_GDCL_FILE_READER_H