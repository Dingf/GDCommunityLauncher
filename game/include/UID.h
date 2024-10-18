#ifndef INC_GDCL_GAME_UID_H
#define INC_GDCL_GAME_UID_H

#include <memory>
#include <string>
#include <cpprest/json.h>
#include "FileReader.h"
#include "FileWriter.h"

class UID16
{
    public:
        UID16();
        UID16(EncodedFileReader* reader);
        UID16(const UID16& id) { *this = id; }

        UID16& operator=(const UID16& id);

        char& operator[](uint8_t index) { return _data[index]; }

        void WriteUID16(EncodedFileWriter* writer);

        std::string ToString() const;

        web::json::value ToJSON() const;

    private:
        char _data[16];
};

#endif//INC_GDCL_GAME_UID_H