#ifndef INC_GDCL_DLL_GAME_HANDLER_H
#define INC_GDCL_DLL_GAME_HANDLER_H

#include <string>
#include <vector>
#include "HookManager.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "WindowsAPI.h"
#include "DllClient.h"
#include "Log.h"

class GameHandler
{
    private:
        GameHandler();
        ~GameHandler();
        GameHandler(GameHandler&) = delete;
        void operator=(const GameHandler&) = delete;

        friend bool InitializeModules();

        static GameHandler* GetInstance();
};

#endif//INC_GDCL_DLL_GAME_HANDLER_H