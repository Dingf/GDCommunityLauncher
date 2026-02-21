#include <filesystem>
#include "GameAPI/TriggerToken.h"
#include "SeasonClient.h"
#include "GameHandler.h"
#include "URI.h"

bool HandleParticipationToken(const std::string& tokenString)
{
    std::string seasonToken = spClient->GetActiveSeasonToken();
    if (tokenString == seasonToken)
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
        spClient->SetActiveCharacter(GameAPI::GetPlayerName(mainPlayer));
        return true;
    }
    return false;
}

bool HandleUnlockToken(const std::string& tokenString)
{
    if ((tokenString == "unlock_all_diff") && (spClient->IsPlayingSeason()))
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
    // TODO: Make this work with websockets
    /*if ((tokenString.find("gdl_", 0) == 0) && (spClient->IsPlayingSeason()))
    {
        void* mainPlayer = GameAPI::GetMainPlayer();

        // Otherwise if it's a season token, pass it along to the server and update the points/rank
        URI endpoint = spClient->GetServerGameURL() / "Season" / "participant" / std::to_string(spClient->GetCurrentParticipantID()) / "quest-tag" / tokenString;
        endpoint.AddParam("branch", spClient->GetBranchName());

        web::http::http_request request(web::http::methods::POST);

        web::json::value requestBody;
        requestBody[U("level")] = EngineAPI::GetPlayerLevel();
        requestBody[U("currentDifficulty")] = GameAPI::GetGameDifficulty();
        requestBody[U("maxDifficulty")] = GameAPI::GetPlayerMaxDifficulty(mainPlayer);
        request.set_body(requestBody);

        std::string bearerToken = "Bearer " + spClient->GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return true;
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](concurrency::task<bool> task)
        {
            try
            {
                if (task.get())
                    spClient->UpdateSeasonStanding();
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to update quest tag: %", ex.what());
            }
        });
        return true;
    }*/
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
        std::string modName = EngineAPI::GetModName();
        void* mainPlayer = GameAPI::GetMainPlayer();

        if ((modName.empty()) && (mainPlayer) && (spClient->IsInActiveSeason()))
        {
            std::string tokenString = token;
            for (char& c : tokenString)
                c = std::tolower(c);

            // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
            if (((EngineAPI::IsMultiplayer()) || (GameAPI::IsCloudStorageEnabled())) && (tokenString != "received_start_items") && (tokenString != spClient->GetActiveSeasonToken()))
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