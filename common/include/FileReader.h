#ifndef INC_GDCL_FILE_READER_H
#define INC_GDCL_FILE_READER_H

#include <memory>
#include <string>

class FileReader
{
    public:
        virtual ~FileReader()
        {
            if (_buffer != nullptr)
            {
                delete[] _buffer;
                _buffer = nullptr;
            }
            _bufferPos = 0;
            _bufferSize = 0;
            _filename = {};
        }

        static std::shared_ptr<FileReader> Open(const std::string& filename)
        {
            FILE* file;
            if (fopen_s(&file, filename.c_str(), "rb") != 0)
                return nullptr;

            std::shared_ptr<FileReader> reader(new FileReader(file));
            reader->_filename = filename;
            return reader;
        }

        std::string GetFilename() const { return _filename; }
        int64_t GetPosition() const { return _bufferPos; }

        void SetPosition(int64_t position) { _bufferPos = position; }

        virtual float ReadFloat()
        {
            if (_bufferPos + 4 <= _bufferSize)
            {
                uint32_t val = (uint32_t)_buffer[_bufferPos] | ((uint32_t)_buffer[_bufferPos + 1] << 8) | ((uint32_t)_buffer[_bufferPos + 2] << 16) | ((uint32_t)_buffer[_bufferPos + 3] << 24);
                val += 4;
                return reinterpret_cast<float&>(val);
            }
            return 0.0f;
        }

        virtual uint8_t ReadInt8()
        {
            if (_bufferPos + 1 <= _bufferSize)
            {
                return _buffer[_bufferPos++];
            }
            return 0;
        }

        virtual uint16_t ReadInt16()
        {
            if (_bufferPos + 2 <= _bufferSize)
            {
                uint16_t val = (uint16_t)_buffer[_bufferPos] | ((uint16_t)_buffer[_bufferPos + 1] << 8);
                _bufferPos += 2;
                return val;
            }
            return 0;
        }

        virtual uint32_t ReadInt32()
        {
            if (_bufferPos + 4 <= _bufferSize)
            {
                uint32_t val = (uint32_t)_buffer[_bufferPos] | ((uint32_t)_buffer[_bufferPos + 1] << 8) | ((uint32_t)_buffer[_bufferPos + 2] << 16) | ((uint32_t)_buffer[_bufferPos + 3] << 24);
                _bufferPos += 4;
                return val;
            }
            return 0;
        }

        virtual std::string ReadString()
        {
            uint32_t length = ReadInt32();
            if ((length == 0) || (_bufferPos + length > _bufferSize))
                return {};

            std::string str(length, '\0');
            for (uint32_t i = 0; i < length; ++i)
            {
                str[i] = (char)_buffer[_bufferPos++];
            }
            return str;
        }

        virtual std::wstring ReadWideString()
        {
            uint32_t length = ReadInt32();
            if ((length == 0) || (_bufferPos + length * 2 > _bufferSize))
                return {};

            std::wstring str(length, '\0');
            for (uint32_t i = 0; i < length; ++i)
            {
                uint8_t c1 = _buffer[_bufferPos++];
                uint8_t c2 = _buffer[_bufferPos++];
                str[i] = (wchar_t)(((uint16_t)c2 << 8) | c1);
            }
            return str;
        }

    protected:
        FileReader(FILE* file) : _bufferPos(0)
        {
            fseek(file, 0, SEEK_END);
            _bufferSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            _buffer = new uint8_t[_bufferSize];
            fread(_buffer, 1, _bufferSize, file);
            fclose(file);
        }

        std::string _filename;

        uint8_t* _buffer;
        int64_t _bufferSize;
        int64_t _bufferPos;
};

class EncodedFileReader : public FileReader
{
    public:
        static std::shared_ptr<EncodedFileReader> Open(const std::string& filename)
        {
            FILE* file;
            if (fopen_s(&file, filename.c_str(), "rb") != 0)
                return nullptr;

            std::shared_ptr<EncodedFileReader> reader(new EncodedFileReader(file));
            reader->_filename = filename;
            return reader;
        }

        float ReadFloat(bool update)
        {
            if (_bufferPos + 4 <= _bufferSize)
            {
                uint32_t val = (uint32_t)_buffer[_bufferPos] | ((uint32_t)_buffer[_bufferPos + 1] << 8) | ((uint32_t)_buffer[_bufferPos + 2] << 16) | ((uint32_t)_buffer[_bufferPos + 3] << 24);
                uint32_t result = val ^ _key;
                _bufferPos += 4;

                if (update)
                    UpdateKey(val);

                return reinterpret_cast<float&>(result);
            }
            return 0.0f;
        }
        float ReadFloat() { return ReadFloat(true); }

        uint8_t ReadInt8(bool update)
        {
            if (_bufferPos + 1 <= _bufferSize)
            {
                uint8_t val = _buffer[_bufferPos++];
                uint32_t result = val ^ _key;

                if (update)
                    _key ^= _table[val];

                return uint8_t(result & 0xFF);
            }
            return 0;
        }
        uint8_t ReadInt8() { return ReadInt8(true); }

        uint16_t ReadInt16(bool update)
        {
            if (_bufferPos + 2 <= _bufferSize)
            {
                uint16_t val = (uint16_t)_buffer[_bufferPos] | ((uint16_t)_buffer[_bufferPos + 1] << 8);
                uint32_t result = val ^ _key;
                _bufferPos += 2;

                if (update)
                    UpdateKey(val);

                return uint16_t(result & 0xFFFF);
            }
            return 0;
        }
        uint16_t ReadInt16() { return ReadInt16(true); }

        uint32_t ReadInt32(bool update)
        {
            if (_bufferPos + 4 <= _bufferSize)
            {
                uint32_t val = (uint32_t)_buffer[_bufferPos] | ((uint32_t)_buffer[_bufferPos + 1] << 8) | ((uint32_t)_buffer[_bufferPos + 2] << 16) | ((uint32_t)_buffer[_bufferPos + 3] << 24);
                uint32_t result = val ^ _key;
                _bufferPos += 4;

                if (update)
                    UpdateKey(val);

                return result;
            }
            return 0;
        }
        uint32_t ReadInt32() { return ReadInt32(true); }

        std::string ReadString()
        {
            uint32_t length = ReadInt32(true);
            if ((length == 0) || (_bufferPos + length > _bufferSize))
                return {};

            std::string str(length, '\0');
            for (uint32_t i = 0; i < length; ++i)
            {
                uint8_t c = _buffer[_bufferPos++];
                uint32_t result = c ^ _key;
                _key ^= _table[c];
                str[i] = (char)(result & 0xFF);
            }
            return str;
        }

        std::wstring ReadWideString()
        {
            uint32_t length = ReadInt32(true);
            if ((length == 0) || (_bufferPos + length * 2 > _bufferSize))
                return {};

            std::wstring str(length, '\0');
            for (uint32_t i = 0; i < length; ++i)
            {
                uint8_t c1 = _buffer[_bufferPos++];
                uint32_t result = c1 ^ _key;
                _key ^= _table[c1];

                uint8_t c2 = _buffer[_bufferPos++];
                result = (((c2 ^ _key) & 0xFF) << 8) | result;
                _key ^= _table[c2];

                str[i] = (wchar_t)(result & 0xFFFF);
            }
            return str;
        }

    private:
        EncodedFileReader(FILE* file) : FileReader(file)
        {
            if (_bufferPos + 4 <= _bufferSize)
            {
                uint32_t val = (uint32_t)_buffer[_bufferPos] | ((uint32_t)_buffer[_bufferPos + 1] << 8) | ((uint32_t)_buffer[_bufferPos + 2] << 16) | ((uint32_t)_buffer[_bufferPos + 3] << 24);
                _bufferPos += 4;
                _key = (val ^= 0x55555555);
                for (uint32_t i = 0; i < 256; ++i)
                {
                    val = ((val >> 1) | (val << 31)) * 39916801;
                    _table[i] = val;
                }
            }
        }

        void UpdateKey(uint32_t val)
        {
            for (uint32_t i = 0; i < 4; ++i)
            {
                uint32_t index = (val & (0xff << (i << 3))) >> (i << 3);
                _key ^= _table[index];
            }
        }

        uint32_t _key;
        uint32_t _table[256];
};



#endif//INC_GDCL_FILE_READER_H