#include <chrono>
#include <functional>
#include "HookManager.h"
#include "ItemLinker.h"

#if _WIN64
constexpr char ITEM_LINK[] = "?GetUIDisplayText@Item@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_ARTIFACT[] = "?GetUIDisplayText@ItemArtifact@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_FORMULA[] = "?GetUIDisplayText@ItemArtifactFormula@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_ENCHANTMENT[] = "?GetUIDisplayText@ItemEnchantment@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_NOTE[] = "?GetUIDisplayText@ItemNote@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_RELIC[] = "?GetUIDisplayText@ItemRelic@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_QUEST_ITEM[] = "?GetUIDisplayText@QuestItem@GAME@@UEBAXPEBVCharacter@2@AEAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
#else
constexpr char ITEM_LINK[] = "?GetUIDisplayText@Item@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_ARTIFACT[] = "?GetUIDisplayText@ItemArtifact@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_FORMULA[] = "?GetUIDisplayText@ItemArtifactFormula@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_ENCHANTMENT[] = "?GetUIDisplayText@ItemEnchantment@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_NOTE[] = "?GetUIDisplayText@ItemNote@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_RELIC[] = "?GetUIDisplayText@ItemRelic@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
constexpr char ITEM_LINK_QUEST_ITEM[] = "?GetUIDisplayText@QuestItem@GAME@@UBEXPBVCharacter@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z";
#endif

ItemLinker& ItemLinker::GetInstance()
{
    static ItemLinker instance;
    return instance;
}

void ItemLinker::LinkItem(void* item)
{
    int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (timestamp - _timestamp >= 10)
    {
        _item = item;
        _timestamp = timestamp;
    }
}

void ItemLinker::HandleItemLink(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

void ItemLinker::HandleItemLinkArtifact(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK_ARTIFACT);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

void ItemLinker::HandleItemLinkFormula(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK_FORMULA);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

void ItemLinker::HandleItemLinkEnchantment(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK_ENCHANTMENT);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

void ItemLinker::HandleItemLinkNote(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK_NOTE);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

void ItemLinker::HandleItemLinkRelic(void* _this, void* character, void* lines)
{
    typedef void(__thiscall* HandleItemLinkProto)(void*, void*, void*);
    HandleItemLinkProto callback = (HandleItemLinkProto)HookManager::GetOriginalFunction("Game.dll", ITEM_LINK_RELIC);
    if (callback)
    {
        ItemLinker::GetInstance().LinkItem(_this);
        callback(_this, character, lines);
    }
}

bool ItemLinker::Initialize()
{
    return (HookManager::CreateHook("Game.dll", ITEM_LINK, &HandleItemLink) &&
            HookManager::CreateHook("Game.dll", ITEM_LINK_ARTIFACT, &HandleItemLinkArtifact) &&
            HookManager::CreateHook("Game.dll", ITEM_LINK_FORMULA, &HandleItemLinkFormula) &&
            HookManager::CreateHook("Game.dll", ITEM_LINK_ENCHANTMENT, &HandleItemLinkEnchantment) &&
            HookManager::CreateHook("Game.dll", ITEM_LINK_NOTE, &HandleItemLinkNote) &&
            HookManager::CreateHook("Game.dll", ITEM_LINK_RELIC, &HandleItemLinkRelic));
}

void ItemLinker::Cleanup()
{
    HookManager::DeleteHook("Game.dll", ITEM_LINK);
    HookManager::DeleteHook("Game.dll", ITEM_LINK_ARTIFACT);
    HookManager::DeleteHook("Game.dll", ITEM_LINK_FORMULA);
    HookManager::DeleteHook("Game.dll", ITEM_LINK_ENCHANTMENT);
    HookManager::DeleteHook("Game.dll", ITEM_LINK_NOTE);
    HookManager::DeleteHook("Game.dll", ITEM_LINK_RELIC);
}

void* ItemLinker::GetLinkedItem()
{
    ItemLinker& itemLinker = ItemLinker::GetInstance();
    return itemLinker._item;
}