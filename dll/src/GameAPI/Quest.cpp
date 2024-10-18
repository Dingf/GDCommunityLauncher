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

void GetQuests(std::vector<void*>& quests, QuestFilter filter)
{
    typedef void (__thiscall* GetQuestsProto)(void*, std::vector<void*>&, QuestFilter);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return;

    GetQuestsProto callback = (GetQuestsProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUESTS);
    void* repository = GetQuestRepository();
    if ((!callback) || (!repository))
        return;

    callback(repository, quests, filter);
}

void* GetQuestByID(uint32_t id)
{
    typedef void* (__thiscall* GetQuestByIDProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetQuestByIDProto callback = (GetQuestByIDProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_BY_ID);
    void* repository = GetQuestRepository();
    if ((!callback) || (!repository))
        return nullptr;

    return callback(repository, id);
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

void* GetQuestTaskByIndex(void* quest, int32_t index)
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

void* GetQuestTaskByID(void* quest, uint32_t id)
{
    typedef void* (__thiscall* GetQuestTaskByIDProto)(void*, uint32_t);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    GetQuestTaskByIDProto callback = (GetQuestTaskByIDProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_TASK_BY_ID);
    if ((!callback) || (!quest))
        return nullptr;

    return callback(quest, id);
}

uint32_t GetQuestTaskState(void* task)
{
    typedef uint32_t (__thiscall* GetQuestTaskStateProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return 0;

    GetQuestTaskStateProto callback = (GetQuestTaskStateProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_TASK_STATE);
    if ((!callback) || (!task))
        return 0;

    return callback(task);
}

bool GetQuestTaskInProgress(void* task, bool unk1)
{
    typedef bool (__thiscall* GetQuestTaskInProgressProto)(void*, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return false;

    GetQuestTaskInProgressProto callback = (GetQuestTaskInProgressProto)GetProcAddress(gameDLL, GAPI_NAME_GET_QUEST_TASK_IN_PROGRESS);
    if ((!callback) || (!task))
        return false;

    return callback(task, unk1);
}

void SetQuestTaskState(void* task, uint32_t state)
{
    *(uint32_t*)((uint8_t*)task + 0x90) = state;
}

void SetQuestTaskInProgress(void* task, uint8_t progress)
{
    *((uint8_t*)task + 0xA0) = progress;
}

}