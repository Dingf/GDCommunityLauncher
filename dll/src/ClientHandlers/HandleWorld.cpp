#include <Windows.h>
#include <cpprest/http_client.h>
#include "LuaAPI.h"
#include "ChatClient.h"
#include "ClientHandlers.h"
#include "DatabaseValues.h"
#include "DungeonDatabase.h"
#include "ItemDatabase.h"
#include "URI.h"
#include "Log.h"

void* prevRegion = nullptr;

void UpdateDungeonData(std::map<std::string, DungeonDatabase::DungeonDBEntry>& database)
{
    Client& client = Client::GetInstance();
    void* state = LuaAPI::GetState();
    if ((client.IsParticipatingInSeason()) && (state))
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
    ItemDatabase& itemDB = ItemDatabase::GetInstance();
    if (!itemDB.IsLoaded())
    {
        HINSTANCE launcherDLL = GetModuleHandle(TEXT("GDCommunityLauncher.dll"));
        HRSRC res = FindResource(launcherDLL, MAKEINTRESOURCE(IDR_ITEMDB), RT_RCDATA);
        if (!res)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load item database from DLL. Item and stash functions will not work properly!");
        }
        else
        {
            HGLOBAL handle = LoadResource(launcherDLL, res);
            DWORD size = SizeofResource(launcherDLL, res);
            char* data = (char*)LockResource(handle);
            itemDB.Load(data, size);
            FreeResource(handle);
        }
    }

    DungeonDatabase& dungeonDB = DungeonDatabase::GetInstance();
    if (!dungeonDB.IsLoaded())
    {
        HINSTANCE launcherDLL = GetModuleHandle(TEXT("GDCommunityLauncher.dll"));
        HRSRC res = FindResource(launcherDLL, MAKEINTRESOURCE(IDR_DUNGEONDB), RT_RCDATA);
        if (!res)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to load dungeon database from DLL. Dungeon scaling will not be displayed properly!");
        }
        else
        {
            HGLOBAL handle = LoadResource(launcherDLL, res);
            DWORD size = SizeofResource(launcherDLL, res);
            char* data = (char*)LockResource(handle);
            dungeonDB.Load(data, size, UpdateDungeonData);
            FreeResource(handle);
        }
    }
}

bool HandleLoadWorld(void* _this, const char* mapName, bool unk1, bool modded)
{
    typedef bool(__thiscall* LoadWorldProto)(void*, const char*, bool, bool);

    LoadWorldProto callback = (LoadWorldProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_LOAD_WORLD);
    if (callback)
    {
        bool result = callback(_this, mapName, unk1, modded);
        if (result)
        {
            Client& client = Client::GetInstance();
            const char* modName = EngineAPI::GetModName();

            client.SetActiveSeason(modName, EngineAPI::GetHardcore());

            // Check the map name to make sure that we are not in the main menu when setting the active season
            if ((modName) && (mapName) && (std::string(mapName) != "levels/mainmenu/mainmenu.map"))
            {
                // Attempt to register the user for the active season
                const SeasonInfo* seasonInfo = client.GetActiveSeason();
                if (seasonInfo)
                {
                    try
                    {
                        URI endpoint = URI(client.GetHostName()) / "api" / "Season" / std::to_string(seasonInfo->_seasonID) / "add-participant" / client.GetUsername();
                        web::http::client::http_client httpClient((utility::string_t)endpoint);
                        web::http::http_request request(web::http::methods::POST);

                        std::string bearerToken = "Bearer " + client.GetAuthToken();
                        request.headers().add(U("Authorization"), bearerToken.c_str());

                        web::http::http_response response = httpClient.request(request).get();
                        if (response.status_code() == web::http::status_codes::OK)
                        {
                            ChatClient& chatClient = ChatClient::GetInstance();

                            web::json::value responseBody = response.extract_json().get();
                            web::json::value participantID = responseBody[U("seasonParticipantId")];
                            web::json::value currentChannel = responseBody[U("currentChannel")];

                            chatClient.SetCurrentChatChannel(currentChannel.as_integer());
                            client.SetParticipantID(participantID.as_integer());
                            client.UpdateSeasonStanding();
                        }
                        else
                        {
                            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                        }
                    }
                    catch (const std::exception& ex)
                    {
                        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to register for season %: %", seasonInfo->_seasonID, ex.what());
                    }
                }
            }
            else
            {
                LoadDatabaseValues();
            }
        }
        return result;
    }
    return false;
}

void HandleSetRegionOfNote(void* _this, void* region)
{
    typedef void(__thiscall* SetRegionOfNoteProto)(void*, void*);

    SetRegionOfNoteProto callback = (SetRegionOfNoteProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_SET_REGION_OF_NOTE);
    if (callback)
    {
        Client& client = Client::GetInstance();
        if ((client.IsParticipatingInSeason()) && (prevRegion != region))
        {
            DungeonDatabase::GetInstance().Update();
            prevRegion = region;
        }

        callback(_this, region);
    }
}

void HandleUnloadWorld(void* _this)
{
    typedef void(__thiscall* UnloadWorldProto)(void*);

    UnloadWorldProto callback = (UnloadWorldProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_UNLOAD_WORLD);
    if (callback)
    {
        callback(_this);

        Client& client = Client::GetInstance();
        const std::wstring& characterName = client.GetActiveCharacterName();
        if ((!EngineAPI::IsMultiplayer()) && (client.IsParticipatingInSeason()) && (!characterName.empty()) && (client.IsOnline()))
        {
            client.SetActiveCharacter({}, false);
        }
    }
}