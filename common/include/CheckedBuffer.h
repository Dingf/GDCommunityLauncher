#ifndef INC_GDCL_CHECKED_BUFFER_H
#define INC_GDCL_CHECKED_BUFFER_H

#include <string>
#include <mutex>
#include "FileBuffer.h"

class CheckedBuffer : public FileBuffer
{
    public:
        CheckedBuffer(size_t size);
        CheckedBuffer(uint8_t* buffer, size_t size);

        std::mutex& GetMutex() { return _mutex; }
        std::string GetChecksum() const { return _checksum; }

        void CopyFromBuffer(uint8_t* buffer, size_t size);

    protected:
        std::mutex  _mutex;
        std::string _checksum;
};

#endif//INC_GDCL_CHECKED_BUFFER_H