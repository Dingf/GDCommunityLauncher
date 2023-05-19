#ifndef INC_GDCL_DLL_GAME_API_ENDLESS_DUNGEON_H
#define INC_GDCL_DLL_GAME_API_ENDLESS_DUNGEON_H

#include <stdint.h>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_SYNC_DUNGEON_PROGRESS[] = "?SyncDungeonProgress@GameEngine@GAME@@QEAAXI@Z";
constexpr char GAPI_NAME_GET_DUNGEON_GENERATOR[] = "?GetDungeonGenerator@GameEngine@GAME@@QEBAPEAVEndlessDungeon_Generator@2@XZ";
constexpr char GAPI_NAME_SR_GET_FLOOR_NUMBER[] = "?GetFloorNumber@EndlessDungeon_Generator@GAME@@QEBAHAEBVRegionId@2@@Z";
constexpr char GAPI_NAME_SR_RESOLVE_EQUATION_VAR[] = "?ResolveEquationVariable@EndlessDungeon_Generator@GAME@@UEBA_NPEBDAEAM@Z";
constexpr char GAPI_NAME_IS_BOSS_REGION[] = "?IsBossRegion@EndlessDungeon_Generator@GAME@@QEBA_NAEBVRegionId@2@@Z";
#else
constexpr char GAPI_NAME_SYNC_DUNGEON_PROGRESS[] = "?SyncDungeonProgress@GameEngine@GAME@@QAEXI@Z";
constexpr char GAPI_NAME_GET_DUNGEON_GENERATOR[] = "?GetDungeonGenerator@GameEngine@GAME@@QBEPAVEndlessDungeon_Generator@2@XZ";
constexpr char GAPI_NAME_SR_GET_FLOOR_NUMBER[] = "?GetFloorNumber@EndlessDungeon_Generator@GAME@@QBEHABVRegionId@2@@Z";
constexpr char GAPI_NAME_SR_RESOLVE_EQUATION_VAR[] = "?ResolveEquationVariable@EndlessDungeon_Generator@GAME@@UBE_NPBDAAM@Z";
constexpr char GAPI_NAME_IS_BOSS_REGION[] = "?IsBossRegion@EndlessDungeon_Generator@GAME@@QBE_NABVRegionId@2@@Z";
#endif

void* GetDungeonGenerator();
int32_t GetCurrentChunk();
uint32_t GetCurrentShard();
bool IsBossChunk();

}

#endif//INC_GDCL_DLL_GAME_API_ENDLESS_DUNGEON_H