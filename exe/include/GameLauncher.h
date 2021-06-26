#ifndef INC_GDCL_EXE_GAME_LAUNCHER_H
#define INC_GDCL_EXE_GAME_LAUNCHER_H

#include <string>
#include <filesystem>
#include "Client.h"

namespace GameLauncher
{
    bool CreateLoginDialog();

    bool LaunchProcess(const Client& client, const std::filesystem::path& exePath, const std::filesystem::path& dllPath);
};

#endif//INC_GDCL_EXE_GAME_LAUNCHER_H