#include <Windows.h>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ItemDatabaseValues.h"
#include "ItemDatabase.h"
#include "URI.h"
#include "Log.h"

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
            ItemDatabase& database = ItemDatabase::GetInstance();
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
                            web::json::value responseBody = response.extract_json().get();
                            web::json::value participantID = responseBody[U("seasonParticipantId")];
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
            else if (!database.IsLoaded())
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
                    database.Load(data, size);
                    FreeResource(handle);
                }
            }
        }
        return result;
    }
    return false;
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