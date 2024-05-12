#include "EngineAPI.h"
#include "GameAPI.h"
#include "EventManager.h"
#include "DeathRecap.h"
#include "Log.h"

DeathRecap::DeathRecapEntry::DeathRecapEntry(GameAPI::CombatAttributeType type, void* attacker, void* target, void* skill, float damage)
{
    _time = EngineAPI::GetGameTime();
    _type = type;
    _attackerID = attacker ? EngineAPI::GetObjectID(attacker) : 0;
    _attackerTag = EngineAPI::GetActorDescriptionTag(attacker);
    _skillTag = GameAPI::GetSkillNameTag(skill);
    _damage = damage;
    _attackerOA = attacker ? GameAPI::GetCharacterOA(attacker) : 0.0f;
    _targetDA = target ? GameAPI::GetCharacterDA(target) : 0.0f;
    _targetHealth = target ? (float)GameAPI::GetCurrentLife(target) : 0.0f;
    _targetResist = 0.0f;
}

DeathRecap::DeathRecap()
{
    EventManager::Subscribe(GDCL_EVENT_WORLD_POST_LOAD, &DeathRecap::OnWorldPostLoad);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD, &DeathRecap::OnWorldPreUnload);
    EventManager::Subscribe(GDCL_EVENT_APPLY_DAMAGE, &DeathRecap::OnApplyDamage);
}

DeathRecap::~DeathRecap()
{
    UnloadTextures();
}

DeathRecap& DeathRecap::GetInstance()
{
    static DeathRecap instance;
    return instance;
}

void DeathRecap::LoadTextures()
{
    const std::vector<std::string> textureNames = 
    {
        "ui/generic/background_bordercornerupperleft.tex",
        "ui/generic/background_borderedgetop.tex",
        "ui/generic/background_bordercornerupperright.tex",
        "ui/generic/background_borderedgeleft.tex",
        "ui/generic/background_borderfiller.tex",
        "ui/generic/background_borderedgeright.tex",
        "ui/generic/background_bordercornerbottomleft.tex",
        "ui/generic/background_borderedgebottom.tex",
        "ui/generic/background_bordercornerbottomright.tex",
    };

    for (const std::string& textureName : textureNames)
    {
        void* texture = EngineAPI::LoadTexture(textureName);
        _textures.push_back(texture);
    }
}

void DeathRecap::UnloadTextures()
{
    for (void* texture : _textures)
    {
        if (texture)
            EngineAPI::UnloadTexture(texture);
    }
    _textures.clear();
}

void DeathRecap::CleanupEntries()
{
    int32_t currentTime = EngineAPI::GetGameTime();
    for (auto it = _entries.begin(); it != _entries.end(); ++it)
    {
        int32_t timeDiff = currentTime - it->_time;
        if (timeDiff >= 10000)
            it = _entries.erase(it);
        else
            break;
    }
}

void DrawBox(const std::vector<void*>& textures, float x, float y, float w, float h, EngineAPI::Color color = EngineAPI::Color::WHITE)
{
    if ((x > 0.0f) && (y > 0.0f) && (w > 0.0f) && (h > 0.0f))
    {
        EngineAPI::RenderRect({ x, y, 8, 8 },         { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[0]), color);
        EngineAPI::RenderRect({ x+8, y, w, 8 },       { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[1]), color);
        EngineAPI::RenderRect({ x+w+8, y, 8, 8 },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[2]), color);
        EngineAPI::RenderRect({ x, y+8, 8, h },       { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[3]), color);
        EngineAPI::RenderRect({ x+8, y+8, w, h },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[4]), color);
        EngineAPI::RenderRect({ x+w+8, y+8, 8, h },   { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[5]), color);
        EngineAPI::RenderRect({ x, y+h+8, 8, 8 },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[6]), color);
        EngineAPI::RenderRect({ x+8, y+h+8, w, 8 },   { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[7]), color);
        EngineAPI::RenderRect({ x+w+8, y+h+8, 8, 8 }, { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(textures[8]), color);
    }
}

void DeathRecap::Render()
{
    if (!_textures.empty())
    {
        uint32_t screenWidth = 1920;
        uint32_t screenHeight = 1080;

        float x = screenWidth * 0.2f;
        float y = screenHeight * 0.2f;
        float w = screenWidth * 0.6f;
        float h = screenHeight * 0.6f;

        DrawBox(_textures, x, y, w, h);
        DrawBox(_textures, x+8+screenWidth*0.15f, y+16, w/2-16, h/10-16, { 1.0f, 1.0f, 1.0f, 0.75f });
        DrawBox(_textures, x+16, y+32+h/10, w*0.6f-16, h*0.9f-48, { 1.0f, 1.0f, 1.0f, 0.75f });
        DrawBox(_textures, x+w*0.6f+32, y+32+h/10, w*0.4f-48, h*0.9f-48, { 1.0f, 1.0f, 1.0f, 0.75f });

        /*EngineAPI::RenderRect({x, y, 8, 8}, {0, 0, 8, 8}, EngineAPI::GetRenderTexture(_textures[0]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+8, y, w, 8 },       { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[1]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+w+8, y, 8, 8 },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[2]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x, y+8, 8, h },       { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[3]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+8, y+8, w, h },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[4]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+w+8, y+8, 8, h },   { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[5]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x, y+h+8, 8, 8 },     { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[6]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+8, y+h+8, w, 8 },   { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[7]), EngineAPI::Color::WHITE);
        EngineAPI::RenderRect({ x+w+8, y+h+8, 8, 8 }, { 0, 0, 8, 8 }, EngineAPI::GetRenderTexture(_textures[8]), EngineAPI::Color::WHITE);*/

  
        //

    }
}


void DeathRecap::OnWorldPostLoad(std::string mapName, bool unk1, bool modded)
{
    //if ((EngineAPI::IsMainCampaign()) && (mapName.substr(0, 16) != "levels/mainmenu/"))
        GetInstance().LoadTextures();
}

void DeathRecap::OnWorldPreUnload()
{
    GetInstance().UnloadTextures();
}

void DeathRecap::OnApplyDamage(void* attacker, void* target, float damage, GameAPI::CombatAttributeType type, void* skill)
{
    void* mainPlayer = GameAPI::GetMainPlayer();
    if (target == mainPlayer)
    {
        GetInstance()._entries.emplace_back(type, attacker, target, skill, damage);
    }
}

void DeathRecap::Update()
{
    GetInstance().CleanupEntries();
    //GetInstance().Render();

}