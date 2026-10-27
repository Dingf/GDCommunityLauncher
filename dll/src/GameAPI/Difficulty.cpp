#include <unordered_map>
#include <Windows.h>
#include "EngineAPI/GameInfo.h"
#include "GameAPI.h"

namespace GameAPI
{

std::unordered_map<std::string, Difficulty> _difficultyNameMap =
{
    { "Normal",   GAME_DIFFICULTY_NORMAL },
    { "Elite",    GAME_DIFFICULTY_ELITE },
    { "Ultimate", GAME_DIFFICULTY_ULTIMATE },
};

Difficulty GetGameDifficulty()
{
    typedef Difficulty (__thiscall* GetGameDifficultyProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    GetGameDifficultyProto callback = (GetGameDifficultyProto)GetProcAddress(gameDLL, GAPI_NAME_GET_GAME_DIFFICULTY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return GAME_DIFFICULTY_UNKNOWN;

    return callback(*gameEngine);
}

Difficulty GetGameDifficultyByName(const std::string& difficultyName)
{
    auto it = _difficultyNameMap.find(difficultyName);
    if (it != _difficultyNameMap.end())
    {
        return it->second;
    }
    else
    {
        return GAME_DIFFICULTY_UNKNOWN;
    }
}

std::string GetGameDifficultyName(Difficulty difficulty, uint32_t challenge)
{
    switch (difficulty)
    {
        case GAME_DIFFICULTY_NORMAL:
        {
            if (challenge == 0x01)
                return "Veteran";
            else
                return "Normal";
        }
        case GAME_DIFFICULTY_ELITE:
            return "Elite";
        case GAME_DIFFICULTY_ULTIMATE:
        {
            if (challenge == 0x03)
                return "Ascendant";
            else
                return "Ultimate";
        }
    }
    return "";
}

std::string GetCurrentGameDifficultyName()
{
    return GetGameDifficultyName(GetGameDifficulty(), EngineAPI::GetGameChallenge());
}

bool IsAscendantDifficulty()
{
    typedef bool (__thiscall* IsAscendantDiffucltyProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    IsAscendantDiffucltyProto callback = (IsAscendantDiffucltyProto)GetProcAddress(gameDLL, GAPI_NAME_IS_ASCENDANT_DIFFICULTY);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return false;

    return callback(*gameEngine);
}

}