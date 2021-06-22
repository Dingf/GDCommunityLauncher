#ifndef INC_GDCL_GAME_FACTION_H
#define INC_GDCL_GAME_FACTION_H

#include <memory>
#include <string>
#include "FileReader.h"

enum Factions
{
    FACTION_NONE = 0,
    FACTION_DEVILS_CROSSING = 1,
    FACTION_AETHERIALS = 2,
    FACTION_CHTHONIANS = 3,
    FACTION_CRONLEYS_GANG = 4,
    FACTION_BEASTS = 5,
    FACTION_ROVERS = 6,
    //FACTION_NEUTRAL = 7,
    FACTION_HOMESTEAD = 8,
    //FACTION_CORRIGAN_MINE = 9,
    FACTION_OUTCAST = 10,
    FACTION_DEATHS_VIGIL = 11,
    FACTION_ARKOVIAN_UNDEAD = 12,
    FACTION_BLACK_LEGION = 13,
    FACTION_KYMONS_CHOSEN = 14,
    FACTION_COVEN_OF_UGDENBOG = 15,
    FACTION_BARROWHOLM = 16,
    FACTION_MALMOUTH_RESISTANCE = 17,
    FACTION_AETHERIAL_VANGUARD = 18,
    FACTION_CULT_OF_BYSMIEL = 19,
    FACTION_CULT_OF_DREEG = 20,
    FACTION_CULT_OF_SOLAEL = 21,
    FACTION_ELDRITCH_HORRORS = 22,
};

struct Faction
{
    public:
        Faction() {}
        Faction(EncodedFileReader* reader);

        Factions _factionID;
        uint8_t  _factionIsModified;
        uint8_t  _factionIsUnlocked;
        float    _factionRepValue;
        float    _factionPositiveBoost;
        float    _factionNegativeBoost;
};

#endif//INC_GDCL_GAME_FACTION_H