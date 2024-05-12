#include <filesystem>
#include <cpprest/http_client.h>
#include "GameAPI/TriggerToken.h"
#include "ChatClient.h"
#include "ClientHandler.h"
#include "URI.h"

bool HandleParticipationToken(const std::string& tokenString)
{
    Client& client = Client::GetInstance();
    const SeasonInfo* seasonInfo = client.GetActiveSeason();

    if (tokenString == seasonInfo->_participationToken)
    {
        if (EngineAPI::IsMultiplayer())
        {
            GameAPI::AddDialog(GameAPI::DIALOG_OKAY, true, 0, "tagGDLeagueMultiplayerWarning", true, true);
        }
        else if (GameAPI::IsCloudStorageEnabled())
        {
            GameAPI::AddDialog(GameAPI::DIALOG_OKAY, true, 0, "tagGDLeagueCloudWarning", true, true);
        }

        // If this is a new character, set the character as the active character so that they can participate in the season
        void* mainPlayer = GameAPI::GetMainPlayer();
        client.SetActiveCharacter(GameAPI::GetPlayerName(mainPlayer));
        return true;
    }
    return false;
}

bool HandleUnlockToken(const std::string& tokenString)
{
    Client& client = Client::GetInstance();
    if ((tokenString == "unlock_all_diff") && (client.IsPlayingSeason()))
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
    if ((tokenString.find("gdl_", 0) == 0) && (client.IsPlayingSeason()))
    {
        Client& client = Client::GetInstance();
        void* mainPlayer = GameAPI::GetMainPlayer();

        // Otherwise if it's a season token, pass it along to the server and update the points/rank
        URI endpoint = client.GetServerGameURL() / "Season" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "quest-tag" / tokenString;
        web::http::http_request request(web::http::methods::POST);

        web::json::value requestBody;
        requestBody[U("level")] = EngineAPI::GetPlayerLevel();
        requestBody[U("currentDifficulty")] = GameAPI::GetGameDifficulty();
        requestBody[U("maxDifficulty")] = GameAPI::GetPlayerMaxDifficulty(mainPlayer);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        pplx::create_task([endpoint, request]()
        {
            web::http::client::http_client httpClient((utility::string_t)endpoint);
            return httpClient.request(request).then([](web::http::http_response response)
            {
                if (response.status_code() == web::http::status_codes::OK)
                {
                    Client::GetInstance().UpdateSeasonStanding();
                }
                else
                {
                    Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update quest tag: Server responded with status code %", response.status_code());
                }
            });
        });
        return true;
    }
    return false;
}

typedef bool (*TokenHandler)(const std::string&);
std::vector<TokenHandler> tokenHandlers =
{
    HandleParticipationToken,
    //HandleUnlockToken,
    HandleSeasonPointToken,
};

void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token)
{
    typedef void (__thiscall* BestowTokenProto)(void*, const GameAPI::TriggerToken&);

    BestowTokenProto callback = (BestowTokenProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_BESTOW_TOKEN);
    if (callback)
    {
        Client& client = Client::GetInstance();
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((modName.empty()) && (mainPlayer) && (seasonInfo))
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