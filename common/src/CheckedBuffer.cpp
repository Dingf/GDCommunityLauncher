#include "CheckedBuffer.h"
#include "MD5.h"

CheckedBuffer::CheckedBuffer(size_t size)
{
    _bufferPos = 0;
    _bufferSize = size;
    _checksum = {};
    if (size > 0)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _buffer = new uint8_t[size];
        memset(_buffer, 0, size * sizeof(uint8_t));
    }
    else
    {
        _buffer = nullptr;
    }
}

CheckedBuffer::CheckedBuffer(uint8_t* buffer, size_t size)
{
    _bufferPos = 0;
    _bufferSize = size;
    if (size > 0)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _buffer = new uint8_t[size];
        memcpy(_buffer, buffer, size);
        _checksum = GenerateBufferMD5(buffer, size);
    }
    else
    {
        _buffer = nullptr;
        _checksum = {};
    }
}

void CheckedBuffer::CopyFromBuffer(uint8_t* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(_mutex);
    size = std::min((size_t)_bufferSize, size);
    if (size > 0)
    {
        memcpy(&_buffer[0], buffer, size);
        _checksum = GenerateBufferMD5(buffer, size);
    }
}