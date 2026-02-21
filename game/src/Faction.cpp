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

void to_json(json& j, const Faction& data)
{
    auto it = factionNameLookup.find(data._factionID);
    std::string factionName = (it == factionNameLookup.end()) ? "Unknown" : it->second;

    j = 
    {
        { "FactionID",        data._factionID },
        { "FactionName",      factionName },
        { "IsModified",       data._factionIsModified },
        { "IsUnlocked",       data._factionIsUnlocked },
        { "Reputation",       data._factionRepValue },
        { "PositiveModifier", data._factionPositiveBoost },
        { "NegativeModifier", data._factionNegativeBoost },
    };
}

void from_json(const json& j, Faction& data)
{
    j.at("FactionID")       .get_to(data._factionID);
    j.at("IsModified")      .get_to(data._factionIsModified);
    j.at("IsUnlocked")      .get_to(data._factionIsUnlocked);
    j.at("Reputation")      .get_to(data._factionRepValue);
    j.at("PositiveModifier").get_to(data._factionPositiveBoost);
    j.at("NegativeModifier").get_to(data._factionNegativeBoost);
}