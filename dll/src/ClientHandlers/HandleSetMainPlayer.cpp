#include "ChatConnection.h"
#include "ClientHandler.h"
#include "EventManager.h"
#include "Quest.h"

bool HasParticipationTokenFromAPI(void* mainPlayer, const SeasonInfo* seasonInfo)
{
    for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
    {
        const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, difficulty);
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            std::string token = tokens[i];
            for (char& c : token)
                c = std::tolower(c);

            if (token == seasonInfo->_participationToken)
                return true;
        }
    }
    return false;
}

bool HasParticipationTokenFromFile(const std::wstring& playerName, const SeasonInfo* seasonInfo)
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

                    if (tokenString == seasonInfo->_participationToken)
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

        Client& client = Client::GetInstance();
        void* mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        EventManager::Publish(GDCL_EVENT_SET_MAIN_PLAYER, mainPlayer);

        if ((mainPlayer) && (seasonInfo))
        {
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
            bool hasParticipationToken = GameAPI::PlayerHasToken(mainPlayer, seasonInfo->_participationToken) ||
                                         HasParticipationTokenFromAPI(mainPlayer, seasonInfo) || 
                                         HasParticipationTokenFromFile(playerName, seasonInfo);

            if (hasParticipationToken)
            {
                GameAPI::BestowTokenNow(mainPlayer, seasonInfo->_participationToken);       // Grant the token just in case because the character might have it from another difficulty/mode
                client.SetActiveCharacter(playerName);
            }

            // Initialize the chat window
            EngineAPI::UI::ChatWindow::GetInstance().Initialize();
        }
    }
}