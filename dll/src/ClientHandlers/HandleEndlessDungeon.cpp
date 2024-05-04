#include "ClientHandler.h"

void HandleSyncDungeonProgress(void* _this, int unk1)
{
    typedef void (__thiscall* SyncDungeonProgressProto)(void*, int);

    SyncDungeonProgressProto callback = (SyncDungeonProgressProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SYNC_DUNGEON_PROGRESS);
    if (callback)
    {
        callback(_this, unk1);

        Client& client = Client::GetInstance();
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();

        if ((modName.empty()) && (mainPlayer) && (client.IsPlayingSeason()))
        {
            if ((GameAPI::GetCurrentChunk() == 3) && (GameAPI::IsBossChunk()))
            {
                uint32_t shard = GameAPI::GetCurrentShard();
                // TODO: Notify the server about completing the shard
            }
        }
    }
}