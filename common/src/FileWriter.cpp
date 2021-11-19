#include "FileWriter.h"

FileWriter::FileWriter(size_t size)
{
    _bufferPos = 0;
    _buffer = new uint8_t[size];
    memset(_buffer, 0, size * sizeof(uint8_t));
}

void FileWriter::WriteFloat(float val)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        *(uint32_t*)(&_buffer[_bufferPos]) = reinterpret_cast<uint32_t&>(val);
        _bufferPos += 4;
    }
}

void FileWriter::WriteInt8(uint8_t val)
{
    if (_bufferPos + 1 <= _bufferSize)
    {
        _buffer[_bufferPos++] = val;
    }
}

void FileWriter::WriteInt16(uint16_t val)
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        *(uint16_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 2;
    }
}

void FileWriter::WriteInt32(uint32_t val)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        *(uint32_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 4;
    }
}

void FileWriter::WriteInt64(uint64_t val)
{
    if (_bufferPos + 8 <= _bufferSize)
    {
        *(uint64_t*)(&_buffer[_bufferPos]) = val;
        _bufferPos += 8;
    }
}

void FileWriter::WriteString(const std::string& val)
{
    uint32_t length = (uint32_t)val.size();
    WriteInt32(length);

    if (_bufferPos + length <= _bufferSize)
    {
        for (uint32_t i = 0; i < length; ++i)
        {
            _buffer[_bufferPos++] = val[i];
        }
    }
}

void FileWriter::WriteWideString(const std::wstring& val)
{
    uint32_t length = (uint32_t)(val.size() * 2);
    WriteInt32(length);

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






void EncodedFileWriter::UpdateKey(uint32_t val)
{
    for (uint32_t i = 0; i < 4; ++i)
    {
        uint32_t index = (val & (0xFF << (i << 3))) >> (i << 3);
        _key ^= _table[index];
    }
}

void EncodedFileWriter::WriteFloat(float val, bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        uint32_t intVal = reinterpret_cast<uint32_t&>(val);

        if (update)
            UpdateKey(intVal);

        *(uint32_t*)(&_buffer[_bufferPos]) = intVal ^ _key;
        _bufferPos += 4;
    }
}

void EncodedFileWriter::WriteInt8(uint8_t val, bool update)
{
    if (_bufferPos + 1 <= _bufferSize)
    {
        if (update)
            _key ^= _table[val];

        _buffer[_bufferPos++] = (val ^ _key) & 0xFF;
    }
}

void EncodedFileWriter::WriteInt16(uint16_t val, bool update)
{
    if (_bufferPos + 2 <= _bufferSize)
    {
        if (update)
            UpdateKey(val);

        *(uint16_t*)(&_buffer[_bufferPos]) = (val ^ _key) & 0xFFFF;
        _bufferPos += 2;
    }
}

void EncodedFileWriter::WriteInt32(uint32_t val, bool update)
{
    if (_bufferPos + 4 <= _bufferSize)
    {
        if (update)
            UpdateKey(val);

        *(uint32_t*)(&_buffer[_bufferPos]) = val ^ _key;
        _bufferPos += 4;
    }
}

void EncodedFileWriter::WriteString(std::string val)
{
    uint32_t length = (uint32_t)val.size();
    WriteInt32(length);

    if (_bufferPos + length <= _bufferSize)
    {
        for (uint32_t i = 0; i < length; ++i)
        {
            WriteInt8(val[i]);
        }
    }
}

void EncodedFileWriter::WriteWideString(std::wstring val)
{
    //TODO: implement me
}