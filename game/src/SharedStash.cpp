#include "Log.h"
#include "FileReader.h"
#include "GDDataBlock.h"
#include "SharedStash.h"

SharedStash::SharedStash(EncodedFileReader* reader)
{
    uint32_t fileVersion = reader->ReadInt32();
    uint32_t headerStart = reader->ReadInt32();
    uint32_t headerLength = reader->ReadInt32(false);
    uint32_t dataVersion = reader->ReadInt32();
    if ((fileVersion != 2) || (headerStart != 18) || ((dataVersion != 3) && (dataVersion != 4) && (dataVersion != 5)))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file signature or version is invalid"));

    uint32_t unk1 = reader->ReadInt32(false);

    std::string modName = reader->ReadString();
    // TODO: Disabled while testing, re-enable in the actual implementation
    /*if ((modName.compare("GrimLeagueS02") != 0) && (modName.compare("GrimLeagueS02_HC") != 0))
    {
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified stash file is not using the GrimLeague Season 3 mod."));
    }*/

    // Vanilla = 0x00, AoM = 0x01, FG = 0x02
    //   Since you can't install FG without AoM though, FG essentially has a value of 0x03
    int8_t expansionStatus = 0;
    if (dataVersion == 5)
    {
        expansionStatus = reader->ReadInt8();
        if (expansionStatus != 3)
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file does not have the correct expansion status (requires AoM and FG)"));
    }

    uint32_t numTabs = reader->ReadInt32();
    for (uint32_t i = 0; i < numTabs; ++i)
    {
        ReadStashTab(reader);
    }
}