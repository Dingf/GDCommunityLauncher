#include <filesystem>
#include <cpprest/http_client.h>
#include "ChatClient.h"
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
            uint32_t tokenLength = *((uint32_t*)((uint8_t*)token + 24));
            uint32_t bufferLength = *((uint32_t*)((uint8_t*)token + 32));
            if (tokenLength > 0)
            {
                std::string tokenString;
                if (bufferLength <= 0x0F)
                    tokenString = std::string((const char*)((PULONG_PTR)token + 1));
                else
                    tokenString = std::string(*((const char**)token + 1));

                for (char& c : tokenString)
                    c = std::tolower(c);

                // If this is a new character, set the character as the active character so that they can participate in the season
                if (tokenString == seasonInfo->_participationToken)
                {
                    if (!EngineAPI::IsMultiplayer())
                    {
                        std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
                        client.SetActiveCharacter(playerName, true);

                        pplx::create_task([]()
                        {
                            ChatClient::GetInstance().DisplayWelcomeMessage();
                        });
                    }
                }
                else if ((EngineAPI::IsMultiplayer()) && (tokenString != "received_start_items"))
                {
                    // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
                    return;
                }
                else if ((tokenString.find("gdl_", 0) == 0) && (client.IsParticipatingInSeason()))
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