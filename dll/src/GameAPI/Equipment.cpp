#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{
    
void* GetPlayerEquipment(void* player)
{
    typedef void* (__thiscall* GetPlayerEquipmentProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetPlayerEquipmentProto callback = (GetPlayerEquipmentProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_EQUIPMENT);
    void* playerController = GetPlayerController(player);

    if ((!callback) || (!playerController))
        return nullptr;

    return callback(playerController);
}

uint32_t GetEquippedItemID(void* equipment, EquipLocation slot)
{
    typedef uint32_t (__thiscall* GetEquippedItemIDProto)(void*, EquipLocation);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetEquippedItemIDProto callback = (GetEquippedItemIDProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_EQUIPPED_ITEM);
    if ((!callback) || (!equipment))
        return 0;

    return callback(equipment, slot);
}

bool IsItemEquipped(void* equipment, uint32_t itemID)
{
    typedef bool (__thiscall* IsItemEquippedProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    IsItemEquippedProto callback = (IsItemEquippedProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_IS_ITEM_EQUIPPED);
    if ((!callback) || (!equipment))
        return false;

    return callback(equipment, itemID);
}

}