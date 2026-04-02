#include <random>
#include <Windows.h>
#include "EngineAPI/Engine.h"
#include "EngineAPI/Graphics.h"
#include "GameAPI.h"

namespace GameAPI
{

std::random_device dev;
std::mt19937 random(dev());

ItemReplicaInfo GetItemReplicaInfo(void* item)
{
    typedef void (__thiscall* GetItemReplicaInfoProto)(void*, ItemReplicaInfo&);

    ItemReplicaInfo info;
    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!item))
        return info;

    GetItemReplicaInfoProto callback = (GetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_REPLICA_INFO);
    if (!callback)
        return info;

    callback(item, info);
    return info;
}

void GetItemReplicaInfo(void* item, ItemReplicaInfo& info)
{
    typedef void (__thiscall* GetItemReplicaInfoProto)(void*, ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!item))
        return;

    GetItemReplicaInfoProto callback = (GetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_REPLICA_INFO);
    if (!callback)
        return;

    callback(item, info);
}

void SetItemReplicaInfo(void* item, const ItemReplicaInfo& info)
{
    typedef void (__thiscall* SetItemReplicaInfoProto)(void*, const ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!item))
        return;

    SetItemReplicaInfoProto callback = (SetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_SET_ITEM_REPLICA_INFO);
    if (!callback)
        return;

    callback(item, info);
}

void SetItemVisiblePlayer(void* item, uint32_t playerID)
{
    typedef void (__thiscall* SetItemReplicaInfoProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if ((!gameDLL) || (!item))
        return;

    SetItemReplicaInfoProto callback = (SetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_SET_ITEM_VISIBLE_PLAYER);
    if (!callback)
        return;

    callback(item, playerID);
}

void* CreateItem(const ItemReplicaInfo& info)
{
    typedef void* (__thiscall* CreateItemProto)(const ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    CreateItemProto callback = (CreateItemProto)GetProcAddress(gameDLL, GAPI_NAME_CREATE_ITEM);
    if (!callback)
        return nullptr;

    return callback(info);
}

uint32_t GenerateItemSeed(uint32_t max)
{
    std::uniform_int_distribution<std::mt19937::result_type> dist(0x00000000, max);
    return dist(random);
}

uint32_t GetItemLevel(void* item)
{
    typedef uint32_t (__thiscall* GetItemLevelProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetItemLevelProto callback = (GetItemLevelProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_LEVEL);
    if ((!callback) || (!item))
        return 0;

    return callback(item);
}

ItemClassification GetItemClassification(void* item)
{
    typedef ItemClassification (__thiscall* GetItemLevelProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return ITEM_CLASSIFICATION_NORMAL;

    GetItemLevelProto callback = (GetItemLevelProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_CLASSIFICATION);
    if ((!callback) || (!item))
        return ITEM_CLASSIFICATION_NORMAL;

    return callback(item);
}

void* GetItemBitmap(void* item)
{
    typedef void* (__thiscall* GetItemBitmapProto)(void*);

    //GetItemBitmapProto callback = *(GetItemBitmapProto*)(*((uintptr_t*)item) + 0x3E0);   // Version 1.2.1.1
    GetItemBitmapProto callback = *(GetItemBitmapProto*)(*((uintptr_t*)item) + 0x3F0);     // Version 1.2.1.6
    if (callback)
        return callback(item);

    return nullptr;
}

uint32_t GetItemWidth(void* item)
{
    if (void* texture = GetItemBitmap(item))
    {
        return EngineAPI::GetTextureWidth(texture) / 32;
    }
    return 0;
}

uint32_t GetItemHeight(void* item)
{
    if (void* texture = GetItemBitmap(item))
    {
        return EngineAPI::GetTextureHeight(texture) / 32;
    }
    return 0;
}

ItemType GetItemType(void* item)
{
    typedef ItemType (__thiscall* GetItemTypeProto)();

    //GetItemTypeProto callback = *(GetItemTypeProto*)(*((uintptr_t*)item) + 0x508);   // Version 1.2.1.1
    GetItemTypeProto callback = *(GetItemTypeProto*)(*((uintptr_t*)item) + 0x518);     // Version 1.2.1.6
    if (callback)
        return callback();

    return ITEM_TYPE_DEFAULT;
}

WeaponType GetWeaponType(void* item)
{
    typedef WeaponType (__thiscall* GetWeaponTypeProto)();

    //GetWeaponTypeProto callback = *(GetWeaponTypeProto*)(*((uintptr_t*)item) + 0x658);   // Version 1.2.1.1
    GetWeaponTypeProto callback = *(GetWeaponTypeProto*)(*((uintptr_t*)item) + 0x668);     // Version 1.2.1.6
    if (callback)
        return callback();

    return WEAPON_TYPE_DEFAULT;
}

std::string GetItemNameTag(void* item)
{
    //return *(std::string*)((uintptr_t)item + 0xC08);    // Version 1.2.1.1
    return *(std::string*)((uintptr_t)item + 0xC50);      // Version 1.2.1.6
}

std::string GetItemPrefixTag(void* item)
{
    //return *(std::string*)((uintptr_t)item + 0x778);    // Pre-version 1.2.0.5
    return *(std::string*)((uintptr_t)item + 0x790);      // Version 1.2.0.5
}

std::string GetItemSuffixTag(void* item)
{
    //return *(std::string*)((uintptr_t)item + 0x798);    // Pre-version 1.2.0.5
    return *(std::string*)((uintptr_t)item + 0x7B0);      // Version 1.2.0.5
}

}