#include "FileWriter.h"

FileWriter::FileWriter(size_t size)
{
    _bufferPos = 0;
    _bufferSize = size;
    _buffer = new uint8_t[size];
    memset(_buffer, 0, size * sizeof(uint8_t));
}

FileWriter::FileWriter(uint8_t* buffer, size_t size)
{
    _bufferSize = size;
    _bufferPos = size;
    _buffer = new uint8_t[size];
    memcpy(_buffer, buffer, size);
}

void FileWriter::BufferFloat(float val)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        *(uint32_t*)(&_buffer[_bufferPos]) = reinterpret_cast<uint32_t&>(val);
        _bufferPos += 4;
    }
}

void FileWriter::BufferInt8(uint8_t val)
{
    if (_bufferPos + 1 <= _bufferSize)
    {
        _buffer[_bufferPos++] = val;
    }
}

void FileWriter::BufferInt16(uint16_t val)
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        *(uint16_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 2;
    }
}

void FileWriter::BufferInt32(uint32_t val)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        *(uint32_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 4;
    }
}

void FileWriter::BufferInt64(uint64_t val)
{
    if (_bufferPos + 8 <= _bufferSize)
    {
        *(uint64_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 8;
    }
}

void FileWriter::BufferString(const std::string& val)
{
    uint32_t length = (uint32_t)val.size();
    BufferInt32(length);

    if (_bufferPos + length <= _bufferSize)
    {
        for (uint32_t i = 0; i < length; ++i)
        {
            _buffer[_bufferPos++] = val[i];
        }
    }
}

void FileWriter::BufferWideString(const std::wstring& val)
{
    uint32_t length = (uint32_t)(val.size());
    BufferInt32(length);

    if (_bufferPos + length <= _bufferSize)
    {
        for (uint32_t i = 0; i < (uint32_t)val.size(); ++i)
        {
            uint8_t c1 = val[i] & 0x00FF;
            uint8_t c2 = (val[i] & 0xFF00) >> 8;
            _buffer[_bufferPos++] = c1;
            _buffer[_bufferPos++] = c2;
        }
    }
}

void FileWriter::CopyFromBuffer(uint8_t* buffer, size_t size)
{
    size = std::min((size_t)(_bufferSize - _bufferPos), size);
    memcpy(&_buffer[_bufferPos], buffer, size);
    _bufferPos += size;
}

void FileWriter::WriteToFile(const std::filesystem::path& filename)
{
    std::filesystem::create_directories(filename.parent_path());
    std::ofstream out(filename, std::ofstream::binary | std::ofstream::out);
    if (out.is_open())
    {
        out.write((const char*)_buffer, _bufferSize);
        out.close();
    }
    else
    {
        throw std::runtime_error(std::string("Could not open file ") + filename.u8string() + " for writing");
    }
}

EncodedFileWriter::EncodedFileWriter(size_t size) : FileWriter(size + 4)
{
    uint32_t val = 0x55555555;
    *(uint32_t*)(&_buffer[_bufferPos]) = val;
    _bufferPos += 4;

    _key = (val ^= 0x55555555);
    for (uint32_t i = 0; i < 256; ++i)
    {
        val = ((val >> 1) | (val << 31)) * 39916801;
        _table[i] = val;
    }
}

void EncodedFileWriter::UpdateKey(uint32_t val)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        uint32_t index = (val & (0xFF << (i << 3))) >> (i << 3);
        _key ^= _table[index];
    }
}

void EncodedFileWriter::BufferFloat(float val, bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t intVal = reinterpret_cast<uint32_t&>(val) ^ _key;
        *(uint32_t*)(&_buffer[_bufferPos]) = intVal;
        _bufferPos += 4;

        if (update)
            UpdateKey(intVal);
    }
}

void EncodedFileWriter::BufferInt8(uint8_t val, bool update)
{
    if (_bufferPos + 1 <= _bufferSize)
    {
        val = (val ^ _key) & 0xFF;
        _buffer[_bufferPos++] = val;

        if (update)
            _key ^= _table[val];
    }
}

void EncodedFileWriter::BufferInt16(uint16_t val, bool update)
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        val = (val ^ _key) & 0xFFFF;
        *(uint16_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 2;

        if (update)
            UpdateKey(val);
    }
}

void EncodedFileWriter::BufferInt32(uint32_t val, bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        val = val ^ _key;
        *(uint32_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 4;

        if (update)
            UpdateKey(val);
    }
}

void EncodedFileWriter::BufferString(std::string val)
{
    uint32_t length = (uint32_t)val.size();
    BufferInt32(length);

    if (_bufferPos + length <= _bufferSize)
    {
        for (uint32_t i = 0; i < length; ++i)
        {
            BufferInt8(val[i]);
        }
    }
}

void EncodedFileWriter::BufferWideString(std::wstring val)
{
    //TODO: implement me
}