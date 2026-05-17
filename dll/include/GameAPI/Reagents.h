#ifndef INC_GDCL_DLL_GAME_API_REAGENTS_H
#define INC_GDCL_DLL_GAME_API_REAGENTS_H

#include <string>
#include <map>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_PLAYER_REAGENTS[] = "?GetPlayerReagents@GameEngine@GAME@@QEBAAEBV?$map@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@UReagentData@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_ADD_ITEM_TO_REAGENTS[] = "?AddItemToReagents@GameEngine@GAME@@QEAA_NI@Z";
constexpr char GAPI_NAME_TAKE_ITEM_FROM_REAGENTS[] = "?TakeItemFromReagents@GameEngine@GAME@@QEAAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@H@Z";
#else
constexpr char GAPI_NAME_GET_PLAYER_REAGENTS[] = "?GetPlayerReagents@GameEngine@GAME@@QBEABV?$map@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@UReagentData@GAME@@@mem@@XZ";
constexpr char GAPI_NAME_ADD_ITEM_TO_REAGENTS[] = "?AddItemToReagents@GameEngine@GAME@@QAE_NI@Z";
constexpr char GAPI_NAME_TAKE_ITEM_FROM_REAGENTS[] = "?TakeItemFromReagents@GameEngine@GAME@@QAEHABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@H@Z";
#endif

struct ReagentData
{
    uint32_t _reagentID;
    uint32_t _reagentCount;
};

const std::map<std::string, ReagentData>& GetPlayerReagents();
bool AddItemToReagents(uint32_t itemID);
int32_t TakeItemFromReagents(std::string itemName, uint32_t count);

}

#endif//INC_GDCL_DLL_GAME_API_REAGENTS_H