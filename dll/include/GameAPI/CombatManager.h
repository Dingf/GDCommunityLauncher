#ifndef INC_GDCL_DLL_GAME_COMBAT_MANAGER_H
#define INC_GDCL_DLL_GAME_COMBAT_MANAGER_H

#include <stdint.h>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_COMBAT_MANAGER[] = "?GetCombatManager@Character@GAME@@QEAAAEAVCombatManager@2@XZ";
constexpr char GAPI_NAME_APPLY_DAMAGE[] = "?ApplyDamage@CombatManager@GAME@@QEAA_NMAEBUPlayStatsDamageType@2@W4CombatAttributeType@2@AEBV?$vector@I@mem@@@Z";
constexpr char GAPI_NAME_CALCULATE_OFFENSIVE_ABILITY[] = "?DesignerCalculateOffensiveAbility@CombatManager@GAME@@QEBAMM@Z";
constexpr char GAPI_NAME_CALCULATE_DEFENSIVE_ABILITY[] = "?DesignerCalculateDefensiveAbility@CombatManager@GAME@@QEBAMM@Z";
#else
constexpr char GAPI_NAME_GET_COMBAT_MANAGER[] = "?GetCombatManager@Character@GAME@@QAEAAVCombatManager@2@XZ";
constexpr char GAPI_NAME_APPLY_DAMAGE[] = "?ApplyDamage@CombatManager@GAME@@QAE_NMABUPlayStatsDamageType@2@W4CombatAttributeType@2@ABV?$vector@I@mem@@@Z";
constexpr char GAPI_NAME_CALCULATE_OFFENSIVE_ABILITY[] = "?DesignerCalculateOffensiveAbility@CombatManager@GAME@@QBEMM@Z";
constexpr char GAPI_NAME_CALCULATE_DEFENSIVE_ABILITY[] = "?DesignerCalculateDefensiveAbility@CombatManager@GAME@@QBEMM@Z";
#endif

enum CombatAttributeType : uint32_t
{
    DAMAGE_TYPE_NONE = 0,
    DAMAGE_TYPE_PHYSICAL = 2,
    DAMAGE_TYPE_PIERCE = 4,
    DAMAGE_TYPE_COLD = 5,
    DAMAGE_TYPE_FIRE = 6,
    DAMAGE_TYPE_POISON = 7,
    DAMAGE_TYPE_LIGHTNING = 8,
    DAMAGE_TYPE_VITALITY = 9,
    DAMAGE_TYPE_CHAOS = 10,
    DAMAGE_TYPE_AETHER = 11,
};

void* GetCombatManager(void* character);
float GetCharacterOA(void* character);
float GetCharacterDA(void* character);

}

#endif//INC_GDCL_DLL_GAME_COMBAT_MANAGER_H