#include <Windows.h>
#include "GameAPI.h"

namespace GameAPI
{

Difficulty GetGameDifficulty()
{
    typedef Difficulty (__thiscall* GetGameDifficultyProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    GetGameDifficultyProto callback = (GetGameDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_GAME_DIFFICULTY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_NORMAL;

    return callback(*gameEngine);
}

std::string GetGameDifficultyName(Difficulty difficulty)
{
    switch (difficulty)
    {
        case GAME_DIFFICULTY_NORMAL:
            return "Normal";
        case GAME_DIFFICULTY_ELITE:
            return "Elite";
        case GAME_DIFFICULTY_ULTIMATE:
            return "Ultimate";
    }
    return "";
}

std::string GetGameDifficultyName()
{
    return GetGameDifficultyName(GetGameDifficulty());
}

}