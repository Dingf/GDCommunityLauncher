#include <filesystem>
#include <cpprest/http_client.h>
#include "GameAPI/TriggerToken.h"
#include "ChatClient.h"
#include "ClientHandlers.h"
#include "URI.h"

bool HandleParticipationToken(const std::string& tokenString)
{
    Client& client = Client::GetInstance();
    const SeasonInfo* seasonInfo = client.GetActiveSeason();
    if (tokenString == seasonInfo->_participationToken)
    {
        if (EngineAPI::IsMultiplayer())
        {
            // TODO: Display a popup to the user here
        }
        else if (GameAPI::IsCloudStorageEnabled())
        {
            // TODO: Display a popup to the user here
        }

        // If this is a new character, set the character as the active character so that they can participate in the season
        void* mainPlayer = GameAPI::GetMainPlayer();
        std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
        client.SetActiveCharacter(playerName, true);
        return true;
    }
    return false;
}

bool HandleUnlockToken(const std::string& tokenString)
{
    Client& client = Client::GetInstance();
    if ((tokenString == "unlock_all_diff") && (client.IsParticipatingInSeason()))
    {
        // Unlock all difficulties after obtaining the unlock token
        void* mainPlayer = GameAPI::GetMainPlayer();
        GameAPI::SetPlayerMaxDifficulty(mainPlayer, GameAPI::GAME_DIFFICULTY_ULTIMATE);
        return true;
    }
    return false;
}

bool HandleSeasonPointToken(const std::string& tokenString)
{
    Client& client = Client::GetInstance();
    if ((tokenString.find("gdl_", 0) == 0) && (client.IsParticipatingInSeason()))
    {
        pplx::create_task([tokenString]()
        {
            try
            {
                Client& client = Client::GetInstance();
                void* mainPlayer = GameAPI::GetMainPlayer();

                // Otherwise if it's a season token, pass it along to the server and update the points/rank
                URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetParticipantID()) / "quest-tag" / tokenString;
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
        return true;
    }
    return false;
}

typedef bool (*TokenHandler)(const std::string&);
std::vector<TokenHandler> tokenHandlers =
{
    HandleParticipationToken,
    HandleUnlockToken,
    HandleSeasonPointToken,
};

void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token)
{
    typedef void (__thiscall* BestowTokenProto)(void*, const GameAPI::TriggerToken&);

    BestowTokenProto callback = (BestowTokenProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_BESTOW_TOKEN);
    if (callback)
    {
        Client& client = Client::GetInstance();
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((!modName.empty()) && (mainPlayer) && (seasonInfo))
        {
            std::string tokenString = token;
            for (char& c : tokenString)
                c = std::tolower(c);

            // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
            if (((EngineAPI::IsMultiplayer()) || (GameAPI::IsCloudStorageEnabled())) && (tokenString != "received_start_items") && (tokenString != seasonInfo->_participationToken))
                return;

            for (size_t i = 0; i < tokenHandlers.size(); ++i)
            {
                TokenHandler handler = tokenHandlers[i];
                if (handler(tokenString))
                    break;
            }
        }

        callback(_this, token);
    }
}