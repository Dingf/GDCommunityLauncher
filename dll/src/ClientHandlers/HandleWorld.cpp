#include <Windows.h>
#include "LuaAPI.h"
#include "ClientHandler.h"
#include "DatabaseValues.h"
#include "DungeonDatabase.h"
#include "CraftingDatabase.h"
#include "EventManager.h"
#include "Configuration.h"
#include "URI.h"

void* prevRegion = nullptr;

void UpdateDungeonData(std::map<std::string, DungeonDatabase::DungeonDBEntry>& database)
{
    Client& client = Client::GetInstance();
    void* state = LuaAPI::GetState();
    if ((client.IsPlayingSeason()) && (EngineAPI::IsMainCampaign()) && (state))
    {
        LuaAPI::lua_getglobal(state, "gd");
        LuaAPI::lua_pushstring(state, "GDLeague");
        LuaAPI::lua_gettable(state, -2);
        LuaAPI::lua_pushstring(state, "InfinityKeyDungeon");
        LuaAPI::lua_gettable(state, -2);
        LuaAPI::lua_pushstring(state, "infinity_dungeons");
        LuaAPI::lua_gettable(state, -2);

        LuaAPI::lua_pushnil(state);
        while (LuaAPI::lua_next(state, -2) != 0)
        {
            const char* key = LuaAPI::lua_tostring(state, -2);
            if (key)
            {
                LuaAPI::lua_pushstring(state, "active");
                LuaAPI::lua_gettable(state, -2);
                database[key]._active = LuaAPI::lua_toboolean(state, -1);
                LuaAPI::lua_pop(state, 1);

                LuaAPI::lua_pushstring(state, "current_level");
                LuaAPI::lua_gettable(state, -2);
                database[key]._level = (uint32_t)LuaAPI::lua_tointeger(state, -1);
                LuaAPI::lua_pop(state, 1);
            }
            LuaAPI::lua_pop(state, 1);
        }
        LuaAPI::lua_pop(state, 4);
    }
}

void LoadDatabaseValues()
{
    HINSTANCE launcherDLL = GetModuleHandle(TEXT("GDCommunityLauncher.dll"));

    DungeonDatabase& dungeonDB = DungeonDatabase::GetInstance();
    if (!dungeonDB.IsLoaded())
    {
        if (HRSRC res = FindResource(launcherDLL, MAKEINTRESOURCE(IDR_DUNGEONDB), RT_RCDATA))
        {
            if (HGLOBAL handle = LoadResource(launcherDLL, res))
            {
                DWORD size = SizeofResource(launcherDLL, res);
                char* data = (char*)LockResource(handle);
                dungeonDB.Load(data, size, UpdateDungeonData);
                FreeResource(handle);
            }
        }

        if (!dungeonDB.IsLoaded())
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load dungeon database from DLL. Dungeon scaling will not be displayed properly!");
    }

    CraftingDatabase& craftingDB = CraftingDatabase::GetInstance();
    if (!craftingDB.IsLoaded())
    {
        if (HRSRC res = FindResource(launcherDLL, MAKEINTRESOURCE(IDR_CRAFTINGDB), RT_RCDATA))
        {
            if (HGLOBAL handle = LoadResource(launcherDLL, res))
            {
                DWORD size = SizeofResource(launcherDLL, res);
                char* data = (char*)LockResource(handle);
                craftingDB.Load(data, size);
                FreeResource(handle);
            }
        }

        if (!craftingDB.IsLoaded())
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load crafting database from DLL. Item crafting will not function properly!");
    }
}

void LoadSeasonModAssets(const std::string& seasonName)
{
    std::string sourceName = "mods/" + seasonName + "/resources";
    std::string databaseName = "mods/" + seasonName +"/database/" + seasonName + ".arz";

    EngineAPI::AddFileSystemSource(EngineAPI::PARTITION_MOD_RESOURCES, sourceName, nullptr, true, true, false);
    EngineAPI::LoadDatabase(databaseName);
    EngineAPI::UI::LoadLocalizationTags(true);
}

bool HandleLoadWorld(void* _this, const char* map, bool unk1, bool modded)
{
    typedef bool (__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        Client& client = Client::GetInstance();
        std::string seasonName = client.GetSeasonName();
        std::string mapName = map ? map : "";
        bool isMainMenu = (mapName.substr(0, 16) == "levels/mainmenu/");

        EventManager::Publish(GDCL_EVENT_WORLD_PRE_LOAD, mapName, modded);

        // When loading the base game, we need to initialize the mod first to ensure that the modded map is loaded instead
        if ((EngineAPI::IsMainCampaign()) && (mapName == "levels/world001.map"))
        {
            EngineAPI::InitializeMod(seasonName);
        }

        // Only allow main campaign, Crucible, and main menu maps to be loaded
        if (callback(_this, map, unk1, modded))
        {
            if (EngineAPI::IsMainCampaignOrCrucible())
            {
                LoadSeasonModAssets(seasonName);

                if (isMainMenu)
                {
                    LoadDatabaseValues();
                }
                else
                {
                    LuaAPI::Initialize();
                }

                // Load the mod scripts only in the main campaign
                if (EngineAPI::IsMainCampaign())
                {
                    EngineAPI::LoadLuaFile("Scripts/game/GDL_SR.lua");
                    EngineAPI::LoadLuaFile("Scripts/game/GDL_Quests.lua");
                }
            }
            // Disallow loading of custom games, but still allow main menu to load
            else if (!isMainMenu)
            {
                return false;
            }
        }

        EventManager::Publish(GDCL_EVENT_WORLD_POST_LOAD, mapName, modded);
        return true;
    }
    return false;
}

void HandleSetRegionOfNote(void* _this, void* region)
{
    typedef void (__thiscall* SetRegionOfNoteProto)(void*, void*);

    SetRegionOfNoteProto callback = (SetRegionOfNoteProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_SET_REGION_OF_NOTE);
    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsPlayingSeason()) && (EngineAPI::IsMainCampaign()) && (prevRegion != region))
        {
            DungeonDatabase::GetInstance().Update();
            prevRegion = region;
        }

        callback(_this, region);
    }
}

void HandleUnloadWorld(void* _this)
{
    typedef void (__thiscall* UnloadWorldProto)(void*);

    UnloadWorldProto callback = (UnloadWorldProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_UNLOAD_WORLD);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_WORLD_PRE_UNLOAD);
        callback(_this);

        Client& client = Client::GetInstance();
        if (client.IsPlayingSeason())
        {
            client.SetActiveCharacter({});
            LuaAPI::Cleanup();
        }

        EventManager::Publish(GDCL_EVENT_WORLD_POST_UNLOAD);
    }
}