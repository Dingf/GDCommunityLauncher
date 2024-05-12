#ifndef INC_GDCL_DLL_GAME_API_QUEST_H
#define INC_GDCL_DLL_GAME_API_QUEST_H

#include <vector>
#include <string>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_QUEST_REPOSITORY[] = "?Get@?$Singleton@VQuest2Repository@GAME@@@GAME@@SAPEAVQuest2Repository@2@XZ";
constexpr char GAPI_NAME_GET_QUEST_BY_ID[] = "?GetQuest@Quest2Repository@GAME@@QEBAPEAVQuest2@2@I@Z";
constexpr char GAPI_NAME_GET_QUESTS[] = "?GetQuests@Quest2Repository@GAME@@QEAAXAEAV?$vector@PEAVQuest2@GAME@@@mem@@W4Filter@12@@Z";
constexpr char GAPI_NAME_GET_QUEST_NAME[] = "?GetName@Quest2@GAME@@QEBAAEBV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@XZ";
constexpr char GAPI_NAME_GET_QUEST_NUM_TASKS[] = "?GetNumTasks@Quest2@GAME@@QEBAIXZ";
constexpr char GAPI_NAME_GET_QUEST_TASK_BY_INDEX[] = "?GetTaskByIndex@Quest2@GAME@@QEBAPEAVQuest2Task@2@H@Z";
constexpr char GAPI_NAME_GET_QUEST_TASK_BY_ID[] = "?GetTaskByUid@Quest2@GAME@@QEBAPEAVQuest2Task@2@I@Z";
constexpr char GAPI_NAME_GET_QUEST_TASK_STATE[] = "?GetState@Quest2Task@GAME@@QEBA?AW4State@12@XZ";
constexpr char GAPI_NAME_GET_QUEST_TASK_IN_PROGRESS[] = "?InProgress@Quest2Task@GAME@@QEBA_N_N@Z";
#else
constexpr char GAPI_NAME_GET_QUEST_REPOSITORY[] = "?Get@?$Singleton@VQuest2Repository@GAME@@@GAME@@SAPAVQuest2Repository@2@XZ";
constexpr char GAPI_NAME_GET_QUEST_BY_ID[] = "?GetQuest@Quest2Repository@GAME@@QBEPAVQuest2@2@I@Z";
constexpr char GAPI_NAME_GET_QUESTS[] = "?GetQuests@Quest2Repository@GAME@@QAEXAAV?$vector@PAVQuest2@GAME@@@mem@@W4Filter@12@@Z";
constexpr char GAPI_NAME_GET_QUEST_NAME[] = "?GetName@Quest2@GAME@@QBEABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@XZ";
constexpr char GAPI_NAME_GET_QUEST_NUM_TASKS[] = "?GetNumTasks@Quest2@GAME@@QBEIXZ";
constexpr char GAPI_NAME_GET_QUEST_TASK_BY_INDEX[] = "?GetTaskByIndex@Quest2@GAME@@QBEPAVQuest2Task@2@H@Z";
constexpr char GAPI_NAME_GET_QUEST_TASK_BY_ID[] = "?GetTaskByUid@Quest2@GAME@@QBEPAVQuest2Task@2@I@Z";
constexpr char GAPI_NAME_GET_QUEST_TASK_STATE[] = "?GetState@Quest2Task@GAME@@QBE?AW4State@12@XZ";
constexpr char GAPI_NAME_GET_QUEST_TASK_IN_PROGRESS[] = "?InProgress@Quest2@GAME@@QBE_N_N@Z";
#endif

enum QuestFilter
{
    QUEST_FILTER_ALL = 0,       // All quests in the game, including the ones the player has not encountered yet
    QUEST_FILTER_ACTIVE = 1,    // Quests that the player has encountered but not completed
    QUEST_FILTER_COMPLETED = 2, // Quests that the player has completed
    QUEST_FILTER_UNLOCKED = 3,  // Quests that the player has encountered, regardless of completion
};

void* GetQuestRepository();
void GetQuests(std::vector<void*>& quests, QuestFilter filter);
void* GetQuestByID(uint32_t id);
std::wstring GetQuestName(void* quest);
uint32_t GetQuestNumTasks(void* quest);
void* GetQuestTaskByIndex(void* quest, int32_t index);
void* GetQuestTaskByID(void* quest, uint32_t id);
uint32_t GetQuestTaskState(void* task);
bool GetQuestTaskInProgress(void* task, bool unk1);
void SetQuestTaskState(void* task, uint32_t state);
void SetQuestTaskInProgress(void* task, uint8_t progress);

}

#endif//INC_GDCL_DLL_GAME_API_QUEST_H