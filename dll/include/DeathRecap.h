#ifndef INC_GDCL_DEATH_RECAP_H
#define INC_GDCL_DEATH_RECAP_H

#include <string>
#include <list>
#include "GameAPI.h"

class DeathRecap
{
    public:
        ~DeathRecap();

        static void Initialize() { GetInstance(); }

        static void Update();

    private:
        struct DeathRecapEntry
        {
            DeathRecapEntry(GameAPI::CombatAttributeType type, void* attacker, void* target, void* skill, float damage);

            int32_t _time;
            GameAPI::CombatAttributeType _type;
            uint32_t _attackerID;
            std::string _attackerTag;
            std::string _skillTag;
            float _damage;
            float _attackerOA;
            float _targetDA;
            float _targetHealth;
            float _targetResist;
        };

        DeathRecap();
        DeathRecap(DeathRecap&) = delete;
        void operator=(const DeathRecap&) = delete;

        static DeathRecap& GetInstance();

        void LoadTextures();
        void UnloadTextures();

        void CleanupEntries();

        void Render();

        static void OnWorldPostLoad(std::string mapName, bool unk1, bool modded);
        static void OnWorldPreUnload();
        static void OnApplyDamage(void* attacker, void* target, float damage, GameAPI::CombatAttributeType type, void* skill);

        std::list<DeathRecapEntry> _entries;
        std::vector<void*> _textures;
};


#endif//INC_GDCL_DEATH_RECAP_H