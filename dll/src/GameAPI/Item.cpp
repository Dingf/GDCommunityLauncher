#include <Windows.h>
#include "EngineAPI/GraphicsTexture.h"
#include "GameAPI/GameEngine.h"
#include "GameAPI/Item.h"

namespace GameAPI
{

void GetItemReplicaInfo(void* item, ItemReplicaInfo& info)
{
    typedef void (__thiscall* GetItemReplicaInfoProto)(void*, ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!item))
        return;

    SetItemReplicaInfoProto callback = (SetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_SET_ITEM_REPLICA_INFO);
    if (!callback)
        return;

    callback(item, info);
}

void* CreateItem(const ItemReplicaInfo& info)
{
    typedef void* (__thiscall* CreateItemProto)(const ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    CreateItemProto callback = (CreateItemProto)GetProcAddress(gameDLL, GAPI_NAME_CREATE_ITEM);
    if (!callback)
        return nullptr;

    return callback(info);
}

uint32_t GetItemLevel(void* item)
{
    typedef uint32_t (__thiscall* GetItemLevelProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
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

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    GetItemBitmapProto callback = (GetItemBitmapProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_BITMAP);
    if ((!callback) || (!item))
        return nullptr;

    return callback(item);
}

uint32_t GetItemWidth(void* item)
{
    void* texture = GetItemBitmap(item);
    return EngineAPI::GetTextureWidth(texture) / 32;
}

uint32_t GetItemHeight(void* item)
{
    void* texture = GetItemBitmap(item);
    return EngineAPI::GetTextureHeight(texture) / 32;
}

std::string GetItemNameTag(void* item)
{
    return *(std::string*)((uintptr_t)item + 0xC08);
}

std::string GetItemPrefixTag(void* item)
{
    return *(std::string*)((uintptr_t)item + 0x778);
}

std::string GetItemSuffixTag(void* item)
{
    return *(std::string*)((uintptr_t)item + 0x798);
}

}