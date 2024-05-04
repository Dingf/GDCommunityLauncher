#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

void** GetGameEngineHandle()
{
    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return nullptr;

    return (void**)GetProcAddress(gameDLL, GAPI_NAME_GAME_ENGINE);
}

}