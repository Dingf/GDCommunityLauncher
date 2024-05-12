#include "ClientHandler.h"
#include "EventManager.h"

bool HandleApplyDamage(void* _this, float damage, void* playStatsDamage, GameAPI::CombatAttributeType type, const std::vector<uint32_t>& skills)
{
    typedef bool (__thiscall* ApplyDamageProto)(void*, float, void*, uint32_t, const std::vector<uint32_t>&);

    ApplyDamageProto callback = (ApplyDamageProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_APPLY_DAMAGE);
    if (callback)
    {
        uint32_t attackerID = *(uint32_t*)((uintptr_t)_this + 0x164);
        uint32_t skillID = (skills.size() > 0) ? skills[0] : 0;
        void* target = *(void**)((uintptr_t)_this + 0x08);
        void* attacker = (attackerID != 0) ? EngineAPI::FindObjectByID(attackerID) : nullptr;
        void* skill = (skillID != 0) ? EngineAPI::FindObjectByID(skillID) : nullptr;

        EventManager::Publish(GDCL_EVENT_APPLY_DAMAGE, attacker, target, damage, type, skill);

        return callback(_this, damage, playStatsDamage, type, skills);
    }
    return false;
}