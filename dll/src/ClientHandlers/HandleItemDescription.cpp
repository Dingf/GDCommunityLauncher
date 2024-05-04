#include <regex>
#include "ClientHandler.h"

void TransformTransferAugment(const GameAPI::ItemReplicaInfo& info, std::vector<GameAPI::GameTextLine>& lines)
{
    const static std::regex transferRegex("^([^:]*):([^:]*)$");

    std::smatch prefixMatch;
    std::smatch suffixMatch;

    bool hasPrefix = std::regex_match(info._itemPrefix, prefixMatch, transferRegex);
    bool hasSuffix = std::regex_match(info._itemSuffix, suffixMatch, transferRegex);
    if (hasPrefix || hasSuffix)
    {
        std::string prefixTag = prefixMatch.str(2);
        std::string suffixTag = suffixMatch.str(2);

        std::wstring result;
        if (!prefixTag.empty())
        {
            result += EngineAPI::UI::Localize(prefixTag.c_str());
            result += L" ";
        }
        result += lines[0]._text;
        if (!suffixTag.empty())
        {
            result += L" ";
            result += EngineAPI::UI::Localize(suffixTag.c_str());
        }
        lines[0]._text = result;
    }
}

void TransformVaalAffix(const GameAPI::ItemReplicaInfo& info, std::vector<GameAPI::GameTextLine>& lines)
{
    const static std::regex vaalRegex("^grimleague/items/lootaffixes/ultos/ultos_affix(\\d{2}[a-z]).dbr$");
    const static std::wregex colorRegex(L"\\^[A-Za-z]");

    bool hasPrefix = std::regex_match(info._itemPrefix, vaalRegex);
    bool hasSuffix = std::regex_match(info._itemSuffix, vaalRegex);
    if (hasPrefix || hasSuffix)
    {
        std::wstring result = lines[0]._text;
        lines[0]._text = L"{^R}" + std::regex_replace(result, colorRegex, L"");
    }
}

void TransformSmithAffix(const GameAPI::ItemReplicaInfo& info, std::vector<GameAPI::GameTextLine>& lines)
{
    const static std::regex smithRegex("^grimleague/items/lootaffixes/ultos/ultos_smith(\\d{2}[a-z]).dbr$");
    const static std::wregex colorRegex(L"\\^[A-Za-z]");

    bool hasModifier = std::regex_match(info._itemModifier, smithRegex);
    if (hasModifier)
    {
        std::wstring result = lines[0]._text;
        lines[0]._text = L"{^F}" + std::regex_replace(result, colorRegex, L"");
    }
}

void HandleGetItemDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_ITEM_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(_this);
        TransformTransferAugment(itemInfo, lines);
        TransformVaalAffix(itemInfo, lines);
        TransformSmithAffix(itemInfo, lines);
    }
}

uint32_t GetWeaponType(void* item)
{
    typedef uint32_t (__thiscall* GetItemTypeProto)();

    GetItemTypeProto callback = *(GetItemTypeProto*)(*((uintptr_t*)item) + 0x650);
    if (callback)
        return callback();

    return 0;
}

void HandleGetWeaponDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_WEAPON_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(_this);
        TransformVaalAffix(itemInfo, lines);
        TransformSmithAffix(itemInfo, lines);
    }
}

void HandleGetArmorDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_GET_ARMOR_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(_this);
        TransformVaalAffix(itemInfo, lines);
        TransformSmithAffix(itemInfo, lines);
    }
}