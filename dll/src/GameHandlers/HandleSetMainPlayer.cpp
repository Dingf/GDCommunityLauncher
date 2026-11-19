#include "ChatAPI.h"
#include "GameHandler.h"
#include "EventManager.h"
#include "ServerCache.h"
#include "Quest.h"

inline static bool SearchForParticipationToken(const std::vector<GameAPI::TriggerToken>& tokens, const std::string& participationToken)
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::string token = tokens[i];
        for (char& c : token)
            c = std::tolower(c);

        if (token == participationToken)
            return true;
    }
    return false;
}

static bool HasParticipationTokenFromAPI(void* mainPlayer, const std::string& participationToken)
{
    for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
    {
        if (SearchForParticipationToken(GameAPI::GetPlayerTokens(mainPlayer, difficulty), participationToken))
            return true;
    }

    if (SearchForParticipationToken(GameAPI::GetSurvivalTokens(mainPlayer), participationToken))
        return true;

    return false;
}

static bool HasParticipationTokenFromFile(const std::wstring& playerName, std::string participationToken)
{
    std::filesystem::path characterPath = GameAPI::GetPlayerFolder(playerName);
    if (std::filesystem::is_directory(characterPath))
    {
        for (const auto& it : std::filesystem::recursive_directory_iterator(characterPath))
        {
            Quest questData;
            const std::filesystem::path& filePath = it.path();
            if ((filePath.filename() == "quests.gdd") && (questData.ReadFromFile(filePath)))
            {
                for (const auto& token : questData._tokensBlock._questTokens)
                {
                    std::string tokenString = token;
                    for (char& c : tokenString)
                        c = std::tolower(c);

                    if (tokenString == participationToken)
                        return true;
                }
            }
        }
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to find character data at \"%\"", characterPath);
    }
    return false;
}

void HandleSetMainPlayer(void* _this, uint32_t unk1)
{
    typedef void (__thiscall* SetMainPlayerProto)(void*, uint32_t);

    SetMainPlayerProto callback = (SetMainPlayerProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SET_MAIN_PLAYER);
    if (callback)
    {
        callback(_this, unk1);

        void* mainPlayer = GameAPI::GetMainPlayer();

        EventManager::Publish(GDCL_EVENT_SET_MAIN_PLAYER, mainPlayer);

        const SeasonInfo* seasonInfo = spClient->GetSeasonByType(GameAPI::IsPlayerHardcore(mainPlayer));
        if ((mainPlayer) && (seasonInfo))
        {
            std::string seasonToken = seasonInfo->_participationToken;
            std::wstring characterName = GameAPI::GetPlayerName(mainPlayer);

            bool hasSeasonToken = GameAPI::PlayerHasToken(mainPlayer, seasonToken) ||
                                  HasParticipationTokenFromAPI(mainPlayer, seasonToken) || 
                                  HasParticipationTokenFromFile(characterName, seasonToken) ||
                                  !GameAPI::HasPlayerBeenInGame(mainPlayer);

            if (hasSeasonToken)
            {
                GameAPI::BestowTokenNow(mainPlayer, seasonToken);       // Grant the token just in case because the character might have it from another difficulty/mode or is new
                spClient->SetActiveSeason(seasonInfo->_seasonID);
                EventManager::Publish(GDCL_EVENT_SET_SEASON_PLAYER, mainPlayer, seasonInfo);
            }
            else
            {
                spClient->SetActiveSeason(0);
            }

            if (!spClient->IsOfflineMode())
            {
                if (EngineAPI::IsMultiplayer())
                {
                    GameAPI::AddDialogA(GameAPI::DIALOG_OKAY, true, 0, "tagGDLeagueMultiplayerWarning", true, 0, true, {}, {});
                }
                if (GameAPI::IsCloudStorageEnabled())
                {
                    GameAPI::AddDialogA(GameAPI::DIALOG_OKAY, true, 0, "tagGDLeagueCloudWarning", true, 0, true, {}, {});
                }
                if (!hasSeasonToken)
                {
                    GameAPI::AddDialogA(GameAPI::DIALOG_OKAY, true, 0, "tagGDLeagueNoTokenWarning", true, 0, true, {}, {});
                }
            }
        }
    }
}