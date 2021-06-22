#include "Quest.h"
#include "Log.h"

Quest::Quest(EncodedFileReader* reader)
{
    uint32_t signature = reader->ReadInt32();
    uint32_t fileVersion = reader->ReadInt32();

    if ((signature != 1480803399) || (fileVersion != 2))
        Logger::LogMessage(LOG_LEVEL_ERROR, "The file signature or version is invalid");
}