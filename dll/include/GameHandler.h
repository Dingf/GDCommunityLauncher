#ifndef INC_GDCL_DLL_GAME_HANDLER_H
#define INC_GDCL_DLL_GAME_HANDLER_H

#include <string>
#include <vector>
#include "HookManager.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "WindowsAPI.h"
#include "SeasonClient.h"
#include "Log.h"


class GameHandler
{
    public:
        GameHandler(GameHandler&) = delete;
        void operator=(const GameHandler&) = delete;

        static bool Initialize();

    private:
        GameHandler();
        ~GameHandler();

};

#endif//INC_GDCL_DLL_GAME_HANDLER_H