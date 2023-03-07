#ifndef INC_GDCL_DLL_GAME_API_DIFFICULTY_H
#define INC_GDCL_DLL_GAME_API_DIFFICULTY_H

#include <stdint.h>

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QEBA?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QEBA?AW4GameDifficulty@2@XZ";
#else
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QBE?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_GET_PLAYER_MAX_DIFFICULTY[] = "?GetGreatestDifficultyCompleted@Player@GAME@@QBE?AW4GameDifficulty@2@XZ";
#endif

enum Difficulty : int32_t
{
    GAME_DIFFICULTY_NORMAL,
    GAME_DIFFICULTY_ELITE,
    GAME_DIFFICULTY_ULTIMATE,
};

const GameAPI::Difficulty GAME_DIFFICULTIES[] = { GameAPI::GAME_DIFFICULTY_NORMAL, GameAPI::GAME_DIFFICULTY_ELITE, GameAPI::GAME_DIFFICULTY_ULTIMATE };

Difficulty GetGameDifficulty();
Difficulty GetPlayerMaxDifficulty(PULONG_PTR player);

}

#endif//INC_GDCL_DLL_GAME_API_DIFFICULTY_H