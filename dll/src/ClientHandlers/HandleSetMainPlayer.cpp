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

            if (GameAPI::GetGameDifficulty() != GameAPI::GAME_DIFFICULTY_NORMAL)
            {
                Quest questData;

                std::filesystem::path characterPath = std::filesystem::path(GameAPI::GetBaseFolder()) / "save" / "user" / "_";
                characterPath += playerName;

                if (questData.ReadFromFile(characterPath / "maps_world001.map" / "Normal" / "quests.gdd"))
                {
                    web::json::value questJSON = questData.ToJSON();
                    web::json::array tokensArray = questJSON[U("Tokens")][U("Tokens")].as_array();

                    uint32_t index = 0;
                    for (auto it2 = tokensArray.begin(); it2 != tokensArray.end(); ++it2)
                    {
                        std::string token = JSONString(it2->serialize());
                        if (token == seasonInfo->_participationToken)
                        {
                            hasParticipationToken = true;
                            break;
                        }
                    }
                }
            }

            client.SetActiveCharacter(playerName, hasParticipationToken);

            GameAPI::SendChatMessage(L"Server", L"Welcome to the Grim Dawn Community League!", 2);
            //GameAPI::SendChatMessage(L"Server", L"To send a message to other players in the league, type /global before your message.", 2);
        }
    }
}