#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void* GetQuestRepository()
{
    typedef void* (__thiscall* GetQuestRepositoryProto)();

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetQuestRepositoryProto callback = (GetQuestRepositoryProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_REPOSITORY);

    if (!callback)
        return nullptr;

    return callback();
}

void GetQuests(std::vector<void*>& quests, uint32_t filter)
{
    typedef void* (__thiscall* GetQuestsProto)(void*, std::vector<void*>&, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    GetQuestsProto callback = (GetQuestsProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUESTS);
    void* repository = GetQuestRepository();
    if ((!callback) || (!repository))
        return;

    callback(repository, quests, filter);
}

std::wstring GetQuestName(void* quest)
{
    typedef const std::wstring& (__thiscall* GetQuestNameProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    GetQuestNameProto callback = (GetQuestNameProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_NAME);
    if ((!callback) || (!quest))
        return {};

    return callback(quest);
}

uint32_t GetQuestNumTasks(void* quest)
{
    typedef uint32_t (__thiscall* GetQuestNumTasksProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetQuestNumTasksProto callback = (GetQuestNumTasksProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_NUM_TASKS);
    if ((!callback) || (!quest))
        return 0;

    return callback(quest);
}

void* GetQuestTaskByindex(void* quest, int32_t index)
{
    typedef void* (__thiscall* GetQuestTaskByIndexProto)(void*, int32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetQuestTaskByIndexProto callback = (GetQuestTaskByIndexProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_TASK_BY_INDEX);
    if ((!callback) || (!quest))
        return nullptr;

    return callback(quest, index);
}


}