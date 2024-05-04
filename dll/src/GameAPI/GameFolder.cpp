#include <Windows.h>
#include "EngineAPI/Engine.h"
#include "EngineAPI/GameInfo.h"
#include "GameAPI.h"

namespace GameAPI
{

std::string _prefix;

void SetRootPrefix(const std::string& prefix)
{
    _prefix = prefix;
}

const std::string& GetRootPrefix()
{
    return _prefix;
}

std::filesystem::path GetBaseFolder()
{
    typedef std::string (__thiscall* GetBaseFolderProto)(void*);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
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
    std::filesystem::path result = GetUserSaveFolder() / (EngineAPI::IsMainCampaignOrCrucible() ? "main" : "user") / "_";
    result += playerName;
    return result;
}

std::filesystem::path GetPlayerFolder(void* player)
{
    std::filesystem::path result = GetUserSaveFolder() / (EngineAPI::IsMainCampaignOrCrucible() ? "main" : "user") / "_";
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

std::filesystem::path GetSharedSavePath(GameAPI::SharedSaveType type)
{
    typedef void (__thiscall* GetSharedSavePathProto)(void*, GameAPI::SharedSaveType, std::string&, bool, bool, bool, bool);

    HMODULE gameDLL = GetModuleHandle(TEXT(GAME_DLL));
    if (!gameDLL)
        return {};

    GetSharedSavePathProto callback = (GetSharedSavePathProto)GetProcAddress(gameDLL, GAPI_NAME_GET_SHARED_SAVE_PATH);
    void** gameEngine = GetGameEngineHandle();

    if ((!callback) || (!gameEngine))
        return {};

    std::string modName = EngineAPI::GetModName();
    if (modName == "survivalmode")
        modName.clear();

    std::string result;
    callback(*gameEngine, type, result, !modName.empty(), 0, 0, 0);
    return result;
}

std::filesystem::path GetTransferStashPath()
{
    return GetSharedSavePath(SHARED_SAVE_TRANSFER);
}

}