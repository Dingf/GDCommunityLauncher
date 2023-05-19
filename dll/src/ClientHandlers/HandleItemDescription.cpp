#include <regex>
#include "ClientHandlers.h"

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
    const static std::regex vaalRegex("^grimleague/items/lootaffixes/ultos_prefix(\\d{2}).dbr$");
    const static std::wregex colorRegex(L"\\^[A-Za-z]");

    bool hasPrefix = std::regex_match(info._itemPrefix, vaalRegex);
    bool hasSuffix = std::regex_match(info._itemSuffix, vaalRegex);
    if (hasPrefix || hasSuffix)
    {
        std::wstring result = lines[0]._text;
        lines[0]._text = L"{^R}" + std::regex_replace(result, colorRegex, L"");
    }
}

void HandleGetItemDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_GET_ITEM_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo info;
        GameAPI::GetItemReplicaInfo(_this, info);

        TransformTransferAugment(info, lines);
        TransformVaalAffix(info, lines);
    }
}

void HandleGetItemDescriptionWeapon(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_GET_WEAPON_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo info;
        GameAPI::GetItemReplicaInfo(_this, info);

        TransformVaalAffix(info, lines);
    }
}

void HandleGetItemDescriptionArmor(void* _this, std::vector<GameAPI::GameTextLine>& lines)
{
    typedef void(__thiscall* GetItemDescriptionProto)(void*, std::vector<GameAPI::GameTextLine>&);

    GetItemDescriptionProto callback = (GetItemDescriptionProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_GET_ARMOR_DESCRIPTION);
    if (callback)
    {
        callback(_this, lines);

        GameAPI::ItemReplicaInfo info;
        GameAPI::GetItemReplicaInfo(_this, info);

        TransformVaalAffix(info, lines);
    }
}