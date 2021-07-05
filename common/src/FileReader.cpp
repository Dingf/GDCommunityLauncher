#include "FileReader.h"

FileReader::FileReader(std::ifstream& in) : _bufferPos(0)
{
    in.seekg(0, in.end);
    _bufferSize = in.tellg();
    in.seekg(0, in.beg);

    _buffer = new uint8_t[_bufferSize];
    in.read((char*)_buffer, _bufferSize);
    in.close();
}

FileReader::~FileReader()
{
    if (_buffer != nullptr)
    {
        delete[] _buffer;
        _buffer = nullptr;
    }
    _bufferPos = 0;
    _bufferSize = 0;
}

std::shared_ptr<FileReader> FileReader::Open(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return nullptr;

    std::shared_ptr<FileReader> reader(new FileReader(in));
    return reader;
}

float FileReader::ReadFloat()
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t val = *(uint32_t*)(&_buffer[_bufferPos]);
        val += 4;
        return reinterpret_cast<float&>(val);
    }
    return 0.0f;
}

uint8_t FileReader::ReadInt8()
{
    if (_bufferPos + 1 <= _bufferSize)
    {
        return _buffer[_bufferPos++];
    }
    return 0;
}

uint16_t FileReader::ReadInt16()
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        uint16_t val = *(uint16_t*)(&_buffer[_bufferPos]);
        _bufferPos += 2;
        return val;
    }
    return 0;
}

uint32_t FileReader::ReadInt32()
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t val = *(uint32_t*)(&_buffer[_bufferPos]);
        _bufferPos += 4;
        return val;
    }
    return 0;
}

uint64_t FileReader::ReadInt64()
{
    if (_bufferPos + 8 <= _bufferSize)
    {
        uint64_t val = *(uint64_t*)(&_buffer[_bufferPos]);
        _bufferPos += 8;
        return val;
    }
    return 0;
}

std::string FileReader::ReadString()
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

std::wstring FileReader::ReadWideString()
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

EncodedFileReader::EncodedFileReader(std::ifstream& in) : FileReader(in)
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

void EncodedFileReader::UpdateKey(uint32_t val)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        uint32_t index = (val & (0xff << (i << 3))) >> (i << 3);
        _key ^= _table[index];
    }
}

std::shared_ptr<EncodedFileReader> EncodedFileReader::Open(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ifstream::binary | std::ifstream::in);
    if (!in.is_open())
        return nullptr;

    std::shared_ptr<EncodedFileReader> reader(new EncodedFileReader(in));
    return reader;
}

float EncodedFileReader::ReadFloat(bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t val = *(uint32_t*)(&_buffer[_bufferPos]);
        uint32_t result = val ^ _key;
        _bufferPos += 4;

        if (update)
            UpdateKey(val);

        return reinterpret_cast<float&>(result);
    }
    return 0.0f;
}

uint8_t EncodedFileReader::ReadInt8(bool update)
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

uint16_t EncodedFileReader::ReadInt16(bool update)
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        uint16_t val = *(uint16_t*)(&_buffer[_bufferPos]);
        uint32_t result = val ^ _key;
        _bufferPos += 2;

        if (update)
            UpdateKey(val);

        return uint16_t(result & 0xFFFF);
    }
    return 0;
}

uint32_t EncodedFileReader::ReadInt32(bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t val = *(uint32_t*)(&_buffer[_bufferPos]);
        uint32_t result = val ^ _key;
        _bufferPos += 4;

        if (update)
            UpdateKey(val);

        return result;
    }
    return 0;
}

uint64_t EncodedFileReader::ReadInt64(bool update)
{
    // This is not confirmed to work, need to find an example of this in-game
    if (_bufferPos + 8 <= _bufferSize)
    {
        uint64_t val = *(uint64_t*)(&_buffer[_bufferPos]);
        uint64_t result = val ^ _key;
        _bufferPos += 8;

        if (update)
            UpdateKey(val);

        return result;
    }
    return 0;
}

std::string EncodedFileReader::ReadString()
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

std::wstring EncodedFileReader::ReadWideString()
{
    uint32_t length = ReadInt32(true);
    if ((length == 0) || (_bufferPos + (length * 2) > _bufferSize))
        return {};

    std::wstring str(length, '\0');
    for (uint32_t i = 0; i < length; ++i)
    {
        uint8_t c1 = _buffer[_bufferPos++];
        wchar_t result1 = (c1 ^ _key) & 0xFF;
        _key ^= _table[c1];

        uint8_t c2 = _buffer[_bufferPos++];
        wchar_t result2 = (c2 ^ _key) & 0xFF;
        _key ^= _table[c2];

        str[i] = ((result2 << 8) | result1);
    }
    return str;
}