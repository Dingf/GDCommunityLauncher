#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "URI.h"
#include "Log.h"

void HandleBestowToken(void* _this, void* token)
{
    typedef void(__thiscall* BestowTokenProto)(void*, void*);

    BestowTokenProto callback = (BestowTokenProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN);
    if (callback)
    {
        Client& client = Client::GetInstance();
        const char* modName = EngineAPI::GetModName();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((modName) && (mainPlayer) && (seasonInfo))
        {
            // Sometimes the token appears to be garbage memory, so check the flags to make sure it's valid
            uint32_t tokenFlags = *((uint32_t*)token + 4);
            if (tokenFlags <= 0xFF)
            {
                std::string tokenString = std::string(*((const char**)token + 1));
                if ((EngineAPI::IsMultiplayer()) && (tokenString != "Received_Start_Items"))
                {
                    // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
                    return;
                }
                else if (tokenString == seasonInfo->_participationToken)
                {
                    std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
                    client.SetActiveCharacter(playerName, true, true);
                }
                else if ((tokenString.find("GDL_", 0) == 0) && (client.IsParticipatingInSeason()))
                {
                    pplx::create_task([tokenString]()
                    {
                        try
                        {
                            Client& client = Client::GetInstance();
                            PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();

                            // Otherwise if it's a season token, pass it along to the server and update the points/rank
                            URI endpoint = URI(client.GetHostName()) / "api" / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "quest-tag" / tokenString;
                            web::http::client::http_client httpClient((utility::string_t)endpoint);
                            web::http::http_request request(web::http::methods::POST);

                            web::json::value requestBody;
                            requestBody[U("level")] = EngineAPI::GetPlayerLevel();
                            requestBody[U("currentDifficulty")] = GameAPI::GetGameDifficulty();
                            requestBody[U("maxDifficulty")] = GameAPI::GetPlayerMaxDifficulty(mainPlayer);
                            request.set_body(requestBody);

                            std::string bearerToken = "Bearer " + client.GetAuthToken();
                            request.headers().add(U("Authorization"), bearerToken.c_str());

                            web::http::http_response response = httpClient.request(request).get();
                            if (response.status_code() == web::http::status_codes::OK)
                            {
                                client.UpdateSeasonStanding();
                            }
                            else
                            {
                                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
                            }
                        }
                        catch (const std::exception& ex)
                        {
                            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update quest tag: %", ex.what());
                        }
                    });
                }
            }
        }

        callback(_this, token);
    }
}