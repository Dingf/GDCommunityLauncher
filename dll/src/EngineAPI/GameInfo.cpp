#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

void* GetGameInfo()
{
    typedef void* (__thiscall* GetGameInfoProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetGameInfoProto callback = (GetGameInfoProto)GetProcAddress(engineDLL, EAPI_NAME_GET_GAME_INFO);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback(*engine);
}

uint32_t GetPlayerLevel()
{
    typedef uint32_t(__thiscall* GetPlayerLevelProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    GetPlayerLevelProto callback = (GetPlayerLevelProto)GetProcAddress(engineDLL, EAPI_NAME_GET_PLAYER_LEVEL);
    void* gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return 0;

    return callback(gameInfo);
}

std::wstring GetLevelName()
{
    typedef void(__thiscall* GetLevelNameProto)(void*, std::wstring&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return {};

    GetLevelNameProto callback = (GetLevelNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_LEVEL_NAME);
    void* gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return {};

    std::wstring result;
    callback(gameInfo, result);
    return result;
}

std::string GetModName()
{
    typedef std::string* (__thiscall* GetModNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return {};

    GetModNameProto callback = (GetModNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_MOD_NAME);
    void* gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return {};

    return *callback(gameInfo);
}

bool IsHardcore()
{
    typedef bool(__thiscall* GetIsHardcoreProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    GetIsHardcoreProto callback = (GetIsHardcoreProto)GetProcAddress(engineDLL, EAPI_NAME_GET_HARDCORE);
    void* gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    return callback(gameInfo);
}

bool IsMultiplayer()
{
    typedef bool(__thiscall* GetIsMultiplayerProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetIsMultiplayerProto callback = (GetIsMultiplayerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_IS_MULTIPLAYER);
    void* gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    return callback(gameInfo);
}

bool IsMainCampaign()
{
    std::string modName = GetModName();
    return modName.empty();
}

bool IsMainCampaignOrCrucible()
{
    std::string modName = GetModName();
    return modName.empty() || modName == "survivalmode";
}

}