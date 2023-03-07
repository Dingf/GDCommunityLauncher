#include "GameAPI.h"

namespace GameAPI
{

void GetItemReplicaInfo(void* item, ItemReplicaInfo& info)
{
    typedef void(__thiscall* GetItemReplicaInfoProto)(void*, ItemReplicaInfo&);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!item))
        return;

    GetItemReplicaInfoProto callback = (GetItemReplicaInfoProto)GetProcAddress(gameDLL, GAPI_NAME_GET_ITEM_REPLICA_INFO);
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

    CreateItemProto callback = (CreateItemProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_CREATE_ITEM);
    if (!callback)
        return nullptr;

    return callback(info);
}

}