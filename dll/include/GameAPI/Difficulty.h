#ifndef INC_GDCL_DLL_GAME_API_DIFFICULTY_H
#define INC_GDCL_DLL_GAME_API_DIFFICULTY_H

#include <stdint.h>
#include <string>

namespace GameAPI
{
#if _WIN64
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QEBA?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_IS_ASCENDANT_DIFFICULTY[] = "?IsUltimateVeteran@GameEngine@GAME@@QEBA_NXZ";
#else
constexpr char GAPI_NAME_GET_GAME_DIFFICULTY[] = "?GetGameDifficulty@GameEngine@GAME@@QBE?AW4GameDifficulty@2@XZ";
constexpr char GAPI_NAME_IS_ASCENDANT_DIFFICULTY[] = "?IsUltimateVeteran@GameEngine@GAME@@QBE_NXZ";
#endif

enum Difficulty : int32_t
{
    GAME_DIFFICULTY_UNKNOWN = -1,
    GAME_DIFFICULTY_NORMAL = 0,
    GAME_DIFFICULTY_ELITE = 1,
    GAME_DIFFICULTY_ULTIMATE = 2,
    GAME_DIFFICULTY_MAX,
};

const GameAPI::Difficulty GAME_DIFFICULTIES[] = { GameAPI::GAME_DIFFICULTY_NORMAL, GameAPI::GAME_DIFFICULTY_ELITE, GameAPI::GAME_DIFFICULTY_ULTIMATE };

Difficulty GetGameDifficulty();
Difficulty GetGameDifficultyByName(const std::string& difficultyName);
std::string GetGameDifficultyName(Difficulty difficulty);
std::string GetCurrentGameDifficultyName();
bool IsAscendantDifficulty();

}

#endif//INC_GDCL_DLL_GAME_API_DIFFICULTY_H