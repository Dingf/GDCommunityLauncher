#include "GameHandler.h"

bool HandleCanTransferItem(void* _this)
{
    typedef bool (__thiscall* CanPutItemInTransferProto)(void*);

    CanPutItemInTransferProto callback = (CanPutItemInTransferProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_CAN_TRANSFER_ITEM);
    if (callback)
    {
        return (GameAPI::IsReagentCompatible(_this)) ? true : callback(_this);
    }
    return false;
}

bool HandleCanTransferQuestItem(void* _this)
{
    typedef bool (__thiscall* CanPutQuestInTransferProto)(void*);

    CanPutQuestInTransferProto callback = (CanPutQuestInTransferProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_CAN_TRANSFER_QUEST_ITEM);
    if (callback)
    {
        return (GameAPI::IsReagentCompatible(_this)) ? true : callback(_this);
    }
    return false;
}