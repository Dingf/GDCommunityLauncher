#include <Windows.h>
#include "GameAPI/GameEngine.h"
#include "GameAPI/GameFolder.h"
#include "GameAPI/Player.h"

namespace GameAPI
{

std::string _prefix;

void SetRootPrefix(const std::string& prefix)
{
    _prefix = prefix;
}

std::filesystem::path GetBaseFolder()
{
    typedef std::string (__thiscall* GetBaseFolderProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT("Game.dll"));
    if (!gameDLL)
        return {};

    GetBaseFolderProto callback = (GetBaseFolderProto)GetProcAddress(gameDLL, GAPI_NAME_GET_BASE_FOLDER);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return {};

    return callback(*gameEngine);
}

std::filesystem::path GetUserSaveFolder()
{
    if (!_prefix.empty())
        return GetBaseFolder() / _prefix / "save" / "";
    else
        return GetBaseFolder() / "save" / "";
}

std::filesystem::path GetPlayerFolder(const std::wstring& playerName)
{
    std::filesystem::path result = GetUserSaveFolder() / "user" / "_";
    result += playerName;
    return result;
}

std::filesystem::path GetPlayerFolder(void* player)
{
    std::filesystem::path result = GetUserSaveFolder() / "user" / "_";
    result += GetPlayerName(player);
    return result;
}

std::filesystem::path GetPlayerSaveFile(const std::wstring& playerName)
{
    return GetPlayerFolder(playerName) / "player.gdc";
}

std::filesystem::path GetPlayerSaveFile(void* player)
{
    return GetPlayerFolder(player) / "player.gdc";
}

std::filesystem::path GetTransferStashPath(const std::string& modName, bool hardcore)
{
    std::filesystem::path stashPath = GameAPI::GetUserSaveFolder();
    if (!modName.empty())
        stashPath /= modName;

    if (hardcore)
        stashPath /= "transfer.gsh";
    else
        stashPath /= "transfer.gst";

    return stashPath;
}

}