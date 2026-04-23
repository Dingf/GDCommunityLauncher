#include <Windows.h>
#include "LuaAPI.h"
#include "GameHandler.h"
#include "DatabaseValues.h"
#include "DungeonDatabase.h"
#include "CraftingDatabase.h"
#include "EventManager.h"
#include "Configuration.h"
#include "URI.h"

void* prevRegion = nullptr;

void UpdateDungeonData(std::map<std::string, DungeonDatabase::DungeonDBEntry>& database)
{
    void* state = LuaAPI::GetState();
    if ((spClient->IsPlayingSeasonOrOffline()) && (EngineAPI::IsMainCampaign()) && (state))
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


    // TODO: This will probably need to be overhauled with FoA's crafting system
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

// TODO: Delete this once the tags_gdcl.txt file is bundled into the season mod
void TemporaryLoadTagsFile()
{
    EngineAPI::UI::AddLocalizationTag("tagGDCLVersion=GDCL v{%t0}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLVersionOffline=Offline Mode", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoOffline= {^L}(Offline Mode)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoCloudSave= {^Y}(Disable Cloud Saving)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoMultiplayer= {^Y}(Multiplayer)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoDisconnected= {^R}(Disconnected)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoRankedPoints= {^L}(Rank {%d0} ~ {%d1} points)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLInfoPoints= {^L}({%d0} points)", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLBoundlessDungeonLevel={^O} (Lv{%d0})", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatDefaultName=Server", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatOnline=There are {%d0} users currently online.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatGlobalDisabled=Global chat is disabled. You can enable it by typing \"/global on\".", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatDirectFrom=[From {%t0}]", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatDirectTo=[To {%t0}]", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChallenges01=Challenge overview for {%t0}:", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChallenges02={%t0} challenges for {%t1}:", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChallenges03={%d0} is not a valid challenge category.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChallengePoints=~ {%d0} points", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatColorSuccess=Changed text color to #{%t0}.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatColorFailed=Could not change text color.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatColorInvalid={%t0} is not a valid color. Type \"/h color\" for a list of available color aliases. You can also use a hex code, e.g. #FFFFFF.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChannelJoin=Joined global channel {%d0}.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatChannelInvalid=Invalid channel. The maximum number of channels is {%d0}.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatMute01={%t0} has been added to your mute list.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatMute02=You have muted the following players:", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatMute03=You have not muted any players.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatMute04=You cannot mute yourself.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatUnmute01={%t0} has been removed from your mute list.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatUnmute02=That player is not currently muted.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatServerError01=That player is not online.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatServerError02=That player does not exist.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatHelp01=The following chat commands are available:", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatHelp02=Type /help <command> for more information about a specific chat command.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatHelp03=The list of available color aliases are:", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatHelpError=Command {%t0} was not found.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatTradeNotification=You have {%d0} new trade notification(s).", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatSavedTags=Tags successfully written to {%t0}.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatAnnouncementsEnabled=Server announcements are enabled.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatAnnouncementsDisabled=Server announcements are disabled. You can enable them by typing \"/server on\".", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLChatServerInvalid=\"{%t0}\" is not a valid server command..", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand01=Displays available commands and their usage.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand01Desc=Usage: /h, /help [command]{^n}{^n}Displays a detailed usage message for a chat command. If no arguments are specified, displays all available chat commands.{^n}{^n}    [command] - Specifies the command to display help information on.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand02=Sends a message to global chat.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand02Desc=Usage: /g, /global[channel] [on|off|color] ...{^n}{^n}Sends a message to the current global chat channel. If no arguments are specified, displays the current global chat channel.{^n}{^n}    [channel] - Sets or switches the current global chat channel. Valid values are 1-255.{^n}{^n}    [on/off] - Enables or disables global chat.{^n}{^n}    [color] - Sets the color of global chat to a color alias or a 6-digit hex code. Type \"/h color\" for a list of color aliases.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand03=Displays the number of online users.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand03Desc=Usage: /o, /online{^n}{^n}Displays the number of concurrent online users.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand04=Displays the list of season challenges.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand04Desc=Usage: /c, /challenges[category]{^n}{^n}Displays the user's current challenge progress in the season. If no arguments are specified, displays an overview of all challenge categories.{^n}{^n}    [category] - Displays a list of individual challenges for the specified challenge category.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand05=Blocks all incoming messages from a user.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand05Desc=Usage: /m, /mute [user]{^n}{^n}Blocks all incoming messages from a user. If no arguments are specified, displays the list of users that you have currently muted.{^n}{^n}    [user] - Specifies the username to be blocked.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand06=Unblocks all incoming messages from a user.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand06Desc=Usage: /u, /unmute <user>{^n}{^n}Unblocks a user that was previously blocked, allowing you to see their messages again.{^n}{^n}    <user> - Specifies the username to be unblocked.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand07=Sends a direct message to a user.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand07Desc=Usage: /t, /tell <user> ...{^n}{^n}Sends a direct message to a user.{^n}{^n}    <user> - Specifies the username to send a message to.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand08=Changes server settings.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommand08Desc=Usage: /s, /server [on|off|color]{^n}{^n}Changes server text color or enables/disables server announcements.{^n}{^n}    [on/off] - Enables or disables server announcements. Some announcements, such as maintenance notifications, cannot be disabled.{^n}{^n}    [color] - Sets the color of server chat to a color alias or a 6-digit hex code. Type \"/h color\" for a list of color aliases.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta01=Adds an item directly into the user's inventory.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta01Desc=Usage: /item <dbr_name> <stack_count>{^n}{^n}Adds an item directly into the user's inventory.{^n}{^n}    <dbr_name> - The full path of the item DBR to add.{^n}{^n}    <stack_count> - The stack count of the item. If not specified, this value will be 1.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta02=Levels up the current character.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta02Desc=Usage: /level <amount>{^n}{^n}Levels up the current character.{^n}{^n}    <count> - Specifies the amount of levels to gain. If not specified, this value will be 1. This value can't be negative and won't cause the character to level up beyond the maximum level.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta03=Adds or removes iron bits from the user's inventory.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta03Desc=Usage: /money <amount>{^n}{^n}Adds or removes iron bits from the user's inventory.{^n}{^n}    <amount> - Specifies the amount of iron bits to add. If this value is negative, the amount will be removed instead.{^n}{^n}", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta04=Saves character quest tags.", "text_en/tags_gdcl.txt", true);
    EngineAPI::UI::AddLocalizationTag("tagGDCLCommandBeta04Desc=Usage: /tags{^n}{^n}Saves all quest tags for the current character to a text file.{^n}{^n}", "text_en/tags_gdcl.txt", true);
}

void LoadSeasonModAssets(const std::string& seasonName)
{
    std::string sourceName = "mods/" + seasonName + "/resources";
    std::string databaseName = "mods/" + seasonName +"/database/" + seasonName + ".arz";

    EngineAPI::AddFileSystemSource(EngineAPI::PARTITION_MOD_RESOURCES, sourceName, nullptr, true, true, false);
    EngineAPI::LoadDatabase(databaseName);
    EngineAPI::UI::LoadLocalizationTags(true);
    GameAPI::ReloadDatabase();

    TemporaryLoadTagsFile();
}

bool HandleLoadWorld(void* _this, const char* map, bool unk1, bool modded)
{
    typedef bool (__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        std::string seasonName = spClient->GetSeasonName();
        std::string mapName = map ? map : "";
        bool isMainMenu = (mapName.substr(0, 16) == "levels/mainmenu/");

        EventManager::Publish(GDCL_EVENT_WORLD_PRE_LOAD, mapName, modded);

        // Prevent loading into multiplayer sessions (online mode only)
        if ((EngineAPI::IsMultiplayer()) && (!isMainMenu) && (!spClient->IsOfflineMode()))
        {
            return false;
        }

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
                LoadDatabaseValues();

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
        if ((spClient->IsPlayingSeasonOrOffline()) && (EngineAPI::IsMainCampaign()) && (prevRegion != region))
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

        if (spClient->IsPlayingSeason())
            spClient->SetActiveSeason(0);

        EventManager::Publish(GDCL_EVENT_WORLD_POST_UNLOAD);
    }
}