#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

const std::map<std::string, ReagentData>& GetPlayerReagents()
{
    static std::map<std::string, ReagentData> empty;
    typedef const std::map<std::string, ReagentData>& (__thiscall* GetPlayerReagentsProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return empty;

    GetPlayerReagentsProto callback = (GetPlayerReagentsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_PLAYER_REAGENTS);
    void** gameEngine = GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return empty;

    return callback(*gameEngine);
}

bool AddItemToReagents(uint32_t itemID)
{
    typedef bool (__thiscall* AddItemToReagentsProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    AddItemToReagentsProto callback = (AddItemToReagentsProto)GetProcAddress(gameDLL, GAPI_NAME_ADD_ITEM_TO_REAGENTS);
    void** gameEngine = GameAPI::GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return false;

    return callback(*gameEngine, itemID);
}

int32_t TakeItemFromReagents(std::string itemName, uint32_t count)
{
    typedef int (__thiscall* TakeItemFromReagentsProto)(void*, std::string, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    TakeItemFromReagentsProto callback = (TakeItemFromReagentsProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_TAKE_ITEM_FROM_REAGENTS);
    void** gameEngine = GameAPI::GetGameEngineHandle();
    if ((!callback) || (!gameEngine))
        return 0;

    return callback(*gameEngine, itemName, count);
}

}