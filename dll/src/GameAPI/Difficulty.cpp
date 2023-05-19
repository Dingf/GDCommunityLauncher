#include <Windows.h>
#include "GameAPI/GameEngine.h"
#include "GameAPI/Difficulty.h"
#include "GameAPI/Game.h"

namespace GameAPI
{

Difficulty GetGameDifficulty()
{
    typedef Difficulty (__thiscall* GetGameDifficultyProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return {};

    GetGameDifficultyProto callback = (GetGameDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_GAME_DIFFICULTY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback(*gameEngine);
}

}