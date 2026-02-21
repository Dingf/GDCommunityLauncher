#ifndef INC_GDCL_GAME_UID_H
#define INC_GDCL_GAME_UID_H

#include <memory>
#include <string>
#include "JSON.h"
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

        operator std::string() const;

        friend void to_json(json& j, const UID16& data);
        friend void from_json(const json& j, UID16& data);

    private:
        char _data[16];
};

#endif//INC_GDCL_GAME_UID_H