#include <Windows.h>
#include "EngineAPI/Region.h"
#include "GameAPI/GameEngine.h"
#include "GameAPI/EndlessDungeon.h"
#include "GameAPI/Player.h"

namespace GameAPI
{

void* GetDungeonGenerator()
{
    typedef void* (__thiscall* GetDungeonGeneratorProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    GetDungeonGeneratorProto callback = (GetDungeonGeneratorProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_DUNGEON_GENERATOR);
    void** gameEngine = GameAPI::GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    return callback(*gameEngine);
}

bool IsBossChunk()
{
    typedef bool(__thiscall* IsBossRegionProto)(void*, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    IsBossRegionProto callback = (IsBossRegionProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_IS_BOSS_REGION);
    void* generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return false;

    void* region = EngineAPI::GetEntityRegion(GameAPI::GetMainPlayer());
    void* regionID = EngineAPI::GetRegionID(region);

    return callback(generator, regionID);
}

int32_t GetCurrentChunk()
{
    typedef int32_t(__thiscall* GetFloorNumberProto)(void*, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return 0;

    GetFloorNumberProto callback = (GetFloorNumberProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_SR_GET_FLOOR_NUMBER);
    void* generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return 0;

    void* region = EngineAPI::GetEntityRegion(GameAPI::GetMainPlayer());
    void* regionID = EngineAPI::GetRegionID(region);

    return callback(generator, regionID);
}

uint32_t GetCurrentShard()
{
    float result = 0.0f;
    typedef bool(__thiscall* ResolveEquationVarProto)(void*, const char*, float*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return 0;

    ResolveEquationVarProto callback = (ResolveEquationVarProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_SR_RESOLVE_EQUATION_VAR);
    void* generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return 0;

    callback(generator, "tierDifficulty", &result);
    return (uint32_t)result;
}

}