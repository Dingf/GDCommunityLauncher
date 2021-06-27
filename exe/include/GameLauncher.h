#ifndef INC_GDCL_EXE_GAME_LAUNCHER_H
#define INC_GDCL_EXE_GAME_LAUNCHER_H

#include <filesystem>
#include "Client.h"

namespace GameLauncher
{
    HANDLE LaunchProcess(const std::filesystem::path& exePath, const std::filesystem::path& dllPath);
};

#endif//INC_GDCL_EXE_GAME_LAUNCHER_H