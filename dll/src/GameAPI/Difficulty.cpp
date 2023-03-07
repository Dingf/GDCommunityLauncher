#include "GameAPI.h"

namespace GameAPI
{

Difficulty GetGameDifficulty()
{
    typedef Difficulty(__thiscall* GetGameDifficultyProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return {};

    GetGameDifficultyProto callback = (GetGameDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_GAME_DIFFICULTY);
    PULONG_PTR gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback((LPVOID)*gameEngine);
}

Difficulty GetPlayerMaxDifficulty(PULONG_PTR player)
{
    typedef Difficulty(__thiscall* GetPlayerMaxDifficultyProto)(LPVOID);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if ((!gameDLL) || (!player))
        return GAME_DIFFICULTY_NORMAL;

    GetPlayerMaxDifficultyProto callback = (GetPlayerMaxDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY);
    if (!callback)
        return GAME_DIFFICULTY_NORMAL;

    return callback((LPVOID)player);
}

}