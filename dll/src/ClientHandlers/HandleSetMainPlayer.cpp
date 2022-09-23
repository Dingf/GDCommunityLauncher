#include "ChatClient.h"
#include "ClientHandlers.h"
#include "Quest.h"

void HandleSetMainPlayer(void* _this, uint32_t unk1)
{
    typedef void(__thiscall* SetMainPlayerProto)(void*, uint32_t);

    SetMainPlayerProto callback = (SetMainPlayerProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SET_MAIN_PLAYER);
    if (callback)
    {
        callback(_this, unk1);

        Client& client = Client::GetInstance();
        PULONG_PTR mainPlayer = GameAPI::GetMainPlayer();
        const SeasonInfo* seasonInfo = client.GetActiveSeason();

        if ((mainPlayer) && (seasonInfo))
        {
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
            bool hasParticipationToken = GameAPI::HasToken(mainPlayer, seasonInfo->_participationToken);

            for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
            {
                if (!hasParticipationToken)
                {
                    const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, difficulty);
                    for (size_t i = 0; i < tokens.size(); ++i)
                    {
                        std::string token = tokens[i].GetTokenString();
                        for (char& c : token)
                            c = std::tolower(c);

                        if (token == seasonInfo->_participationToken)
                        {
                            hasParticipationToken = true;
                            break;
                        }
                    }
                }
            }

            client.SetActiveCharacter(playerName, hasParticipationToken);
            pplx::create_task([&client]()
            {
                // Initialize the chat window
                EngineAPI::UI::ChatWindow::GetInstance(true);

                if (client.IsParticipatingInSeason())
                    ChatClient::GetInstance().DisplayWelcomeMessage();
            });
        }
    }
}