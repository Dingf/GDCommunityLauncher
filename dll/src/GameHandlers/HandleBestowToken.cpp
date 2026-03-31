#include <filesystem>
#include "GameAPI/TriggerToken.h"
#include "GameHandler.h"
#include "ServerHandler.h"
#include "URI.h"

// TODO: Move the dialog popups here to somewhere else
/*bool HandleParticipationToken(const std::string& tokenString)
{
    if (spClient->GetActiveSeason() == nullptr)
    {
        // If this is a new character, set the character as the active character so that they can participate in the season
        void* mainPlayer = GameAPI::GetMainPlayer();
        if (const SeasonInfo* seasonInfo = spClient->GetSeasonByType(GameAPI::IsPlayerHardcore(mainPlayer)))
        {
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

                spClient->SetActiveSeason(seasonInfo->_seasonID);
                return true;
            } 
        }
    }
    return false;
}*/

/*bool HandleUnlockToken(const std::string& tokenString)
{
    if ((tokenString == "unlock_all_diff") && (spClient->IsPlayingSeason()))
    {
        // Unlock all difficulties after obtaining the unlock token
        void* mainPlayer = GameAPI::GetMainPlayer();
        GameAPI::SetPlayerMaxDifficulty(mainPlayer, GameAPI::GAME_DIFFICULTY_ULTIMATE);
        return true;
    }
    return false;
}*/

bool HandleSeasonPointToken(const std::string& tokenString)
{
    if ((tokenString.starts_with("gdl_")) && (spClient->IsPlayingSeason()))
    {
        spServer->Send("SaveParticipantTag", tokenString, EngineAPI::GetPlayerLevel(), GameAPI::GetGameDifficulty());
        return true;
    }
    return false;
}

typedef bool (*TokenHandler)(const std::string&);
std::vector<TokenHandler> tokenHandlers =
{
    //HandleParticipationToken,
    //HandleUnlockToken,
    HandleSeasonPointToken,
};

void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token)
{
    typedef void (__thiscall* BestowTokenProto)(void*, const GameAPI::TriggerToken&);

    BestowTokenProto callback = (BestowTokenProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_BESTOW_TOKEN);
    if (callback)
    {
        void* mainPlayer = GameAPI::GetMainPlayer();
        if ((mainPlayer) && (EngineAPI::IsMainCampaign()) && (spClient->IsPlayingSeason()))
        {
            std::string tokenString = token;
            for (char& c : tokenString)
                c = std::tolower(c);

            // Prevent tokens from being updated in multiplayer season games (except for starting item token, to avoid receiving the starting items multiple times)
            if (((EngineAPI::IsMultiplayer()) || (GameAPI::IsCloudStorageEnabled())) && (tokenString != "received_start_items"))
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