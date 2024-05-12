#ifndef INC_GDCL_DLL_GAME_API_ATTRIBUTE_H
#define INC_GDCL_DLL_GAME_API_ATTRIBUTE_H

#include <stdint.h>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_TOTAL_CHAR_ATTRIBUTE[] = "?GetTotalCharAttribute@Character@GAME@@QEBAMW4CharAttributeType@2@@Z";
constexpr char GAPI_NAME_GET_TOTAL_CHAR_MODIFIER[] = "?GetTotalCharModifier@Character@GAME@@QEBAMW4CharAttributeType@2@@Z";
constexpr char GAPI_NAME_GET_CURRENT_LIFE[] = "?GetCurrentLife@Character@GAME@@QEBA?BNXZ";
#else
constexpr char GAPI_NAME_GET_TOTAL_CHAR_ATTRIBUTE[] = "?GetTotalCharAttribute@Character@GAME@@QBEMW4CharAttributeType@2@@Z";
constexpr char GAPI_NAME_GET_TOTAL_CHAR_MODIFIER[] = "?GetTotalCharModifier@Character@GAME@@QBEMW4CharAttributeType@2@@Z";
constexpr char GAPI_NAME_GET_CURRENT_LIFE[] = "?GetCurrentLife@Character@GAME@@QBE?BNXZ";
#endif

// TODO: Figure out the rest of the values
enum CharAttributeType : uint32_t
{
    CHAR_ATTRIBUTE_TYPE_NONE = 0,
    CHAR_ATTRIBUTE_TYPE_PHYSIQUE = 1,
    CHAR_ATTRIBUTE_TYPE_CUNNING = 2,
    CHAR_ATTRIBUTE_TYPE_SPIRIT = 3,
    CHAR_ATTRIBUTE_TYPE_HEALTH = 4,
    CHAR_ATTRIBUTE_TYPE_ENERGY = 5,
    CHAR_ATTRIBUTE_TYPE_HEALTH_REGEN = 6,
    CHAR_ATTRIBUTE_TYPE_ENERGY_REGEN = 7,
    CHAR_ATTRIBUTE_TYPE_HEAL_INCREASE = 11,
    CHAR_ATTRIBUTE_TYPE_MOVEMENT_SPEED = 14,
    CHAR_ATTRIBUTE_TYPE_ATTACK_SPEED = 15,
    CHAR_ATTRIBUTE_TYPE_CASTING_SPEED = 16,
    CHAR_ATTRIBUTE_TYPE_OA_FLAT = 21,
    CHAR_ATTRIBUTE_TYPE_OA_PCT = 22,
    CHAR_ATTRIBUTE_TYPE_DA_FLAT = 23,
    CHAR_ATTRIBUTE_TYPE_DA_PCT = 24,
};

float GetTotalCharAttribute(void* character, CharAttributeType type);
float GetTotalCharModifier(void* character, CharAttributeType type);
double GetCurrentLife(void* character);

}


#endif//INC_GDCL_DLL_GAME_API_ATTRIBUTE_H