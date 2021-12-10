#ifndef INC_GDCL_FILE_DATA_H
#define INC_GDCL_FILE_DATA_H

#include "FileReader.h"
#include "FileWriter.h"

class FileData
{
    public:
        virtual void Read(EncodedFileReader* reader) = 0;
        virtual void Write(EncodedFileWriter* writer) = 0;
        virtual size_t GetBufferSize() const = 0;
};

#endif//INC_GDCL_BUFFER_DATA_H