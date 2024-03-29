#include <unordered_map>
#include "Faction.h"

const std::unordered_map<Factions, std::string> factionNameLookup =
{
    { FACTION_DEVILS_CROSSING,     "Devil's Crossing" },
    { FACTION_AETHERIALS,          "Aetherials" },
    { FACTION_CHTHONIANS,          "Chthonians" },
    { FACTION_CRONLEYS_GANG,       "Cronley's Gang" },
    { FACTION_BEASTS,              "Beasts" },
    { FACTION_ROVERS,              "Rovers" },
    { FACTION_HOMESTEAD,           "Homestead" },
    { FACTION_OUTCAST,             "The Outcast" },
    { FACTION_DEATHS_VIGIL,        "Order of Death's Vigil" },
    { FACTION_ARKOVIAN_UNDEAD,     "Undead" },
    { FACTION_BLACK_LEGION,        "The Black Legion" },
    { FACTION_KYMONS_CHOSEN,       "Kymon's Chosen" },
    { FACTION_COVEN_OF_UGDENBOG,   "Coven of Ugdenbog" },
    { FACTION_BARROWHOLM,          "Barrowholm" },
    { FACTION_MALMOUTH_RESISTANCE, "Malmouth Resistance" },
    { FACTION_AETHERIAL_VANGUARD,  "Aetherial Vanguard" },
    { FACTION_CULT_OF_BYSMIEL,     "Cult of Bysmiel" },
    { FACTION_CULT_OF_DREEG,       "Cult of Dreeg" },
    { FACTION_CULT_OF_SOLAEL,      "Cult of Solael" },
    { FACTION_ELDRITCH_HORRORS,    "Eldritch Horrors" },
};

void Faction::Read(EncodedFileReader* reader)
{
    _factionIsModified = reader->ReadInt8();
    _factionIsUnlocked = reader->ReadInt8();
    _factionRepValue = reader->ReadFloat();
    _factionPositiveBoost = reader->ReadFloat();
    _factionNegativeBoost = reader->ReadFloat();
}

web::json::value Faction::ToJSON() const
{
    std::string factionName = (factionNameLookup.count(_factionID) > 0) ? factionNameLookup.at(_factionID) : "";

    web::json::value obj = web::json::value::object();
    
    obj[U("FactionID")] = _factionID;
    obj[U("FactionName")] = JSONString(factionName);
    obj[U("IsModified")] = _factionIsModified;
    obj[U("IsUnlocked")] = _factionIsUnlocked;
    obj[U("Reputation")] = _factionRepValue;
    obj[U("PositiveModifier")] = _factionPositiveBoost;
    obj[U("NegativeModifier")] = _factionNegativeBoost;

    return obj;
}