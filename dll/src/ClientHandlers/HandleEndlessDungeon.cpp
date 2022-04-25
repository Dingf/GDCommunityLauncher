#include "ClientHandlers.h"

LPVOID GetDungeonGenerator()
{
    typedef LPVOID(__thiscall* GetDungeonGeneratorProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    GetDungeonGeneratorProto callback = (GetDungeonGeneratorProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_GET_DUNGEON_GENERATOR);
    PULONG_PTR gameEngine = GameAPI::GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return nullptr;

    return callback((LPVOID)*gameEngine);
}

bool IsBossChunk()
{
    typedef bool(__thiscall* IsBossRegionProto)(void*, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return false;

    IsBossRegionProto callback = (IsBossRegionProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_IS_BOSS_REGION);
    LPVOID generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return false;

    LPVOID region = EngineAPI::GetEntityRegion(GameAPI::GetMainPlayer());
    LPVOID regionID = EngineAPI::GetRegionID(region);

    return callback(generator, regionID);
}

int32_t GetCurrentChunk()
{
    typedef int32_t(__thiscall* GetFloorNumberProto)(void*, void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return 0;

    GetFloorNumberProto callback = (GetFloorNumberProto)GetProcAddress(gameDLL, GameAPI::GAPI_NAME_SR_GET_FLOOR_NUMBER);
    LPVOID generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return 0;

    LPVOID region = EngineAPI::GetEntityRegion(GameAPI::GetMainPlayer());
    LPVOID regionID = EngineAPI::GetRegionID(region);

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
    LPVOID generator = GetDungeonGenerator();

    if ((!callback) || (!generator))
        return 0;

    callback(generator, "tierDifficulty", &result);
    return (uint32_t)result;
}

void HandleSyncDungeonProgress(void* _this, int unk1)
{
    typedef void(__thiscall* SyncDungeonProgressProto)(void*, int);

    SyncDungeonProgressProto callback = (SyncDungeonProgressProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SYNC_DUNGEON_PROGRESS);
    if (callback)
    {
        callback(_this, unk1);

        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

        if ((modName) && (mainPlayer) && (client.IsParticipatingInSeason()))
        {
            if ((GetCurrentChunk() == 3) && (IsBossChunk()))
            {
                uint32_t shard = GetCurrentShard();
                // TODO: Notify the server about completing the shard
            }
        }
    }
}