#include "UID.h"

UID16::UID16()
{
    for (uint32_t i = 0; i < 16; ++i)
    {
        _data[i] = 0;
    }
}

UID16::UID16(EncodedFileReader* reader)
{
    for (uint32_t i = 0; i < 16; ++i)
    {
        _data[i] = reader->ReadInt8(true);
    }
}

UID16& UID16::operator=(const UID16& id)
{
    for (uint32_t i = 0; i < 16; ++i)
    {
        _data[i] = id._data[i];
    }
    return *this;
}

std::string UID16::ToString() const
{
    std::string result;
    result.reserve(32);

    for (uint32_t i = 0; i < 16; ++i)
    {
        char high = (_data[i] & 0xF0) >> 4;
        char low = (_data[i] & 0x0F);

        high += (high < 10) ? '0' : ('A' - 10);
        low += (low < 10) ? '0' : ('A' - 10);

        result.push_back(high);
        result.push_back(low);
    }
    return result;
}