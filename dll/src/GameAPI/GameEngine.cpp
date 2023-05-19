#include <Windows.h>
#include "GameAPI/GameEngine.h"

namespace GameAPI
{

void** GetGameEngineHandle()
{
    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return nullptr;

    return (void**)GetProcAddress(gameDLL, GAPI_NAME_GAME_ENGINE);
}

}