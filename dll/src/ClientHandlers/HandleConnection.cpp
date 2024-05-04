#include "ClientHandler.h"

int32_t HandleCreateNewConnection(void* _this, void* unk1, void* unk2, void* unk3)
{
    return 0;
}

void HandleAddNetworkServer(void* _this, void* server, uint32_t unk1)
{
    // Empty to prevent multiplayer while playing on the season
    // If playing offline, this hook won't be called
}