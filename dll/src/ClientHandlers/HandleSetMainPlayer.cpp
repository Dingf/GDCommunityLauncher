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

                for (const auto& it : std::filesystem::recursive_directory_iterator(characterPath))
                {
                    const std::filesystem::path& filePath = it.path();
                    if ((filePath.filename() == "quests.gdd") && (questData.ReadFromFile(filePath)))
                    {
                        web::json::value questJSON = questData.ToJSON();
                        web::json::array tokensArray = questJSON[U("Tokens")][U("Tokens")].as_array();

                        uint32_t index = 0;
                        for (auto it2 = tokensArray.begin(); it2 != tokensArray.end(); ++it2)
                        {
                            std::string token = JSONString(it2->serialize());
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
            }

            client.SetActiveCharacter(playerName, hasParticipationToken);

            pplx::create_task([]()
            {
                EngineAPI::UI::ChatWindow::GetInstance(true);
                GameAPI::SendChatMessage(L"Server", L"Welcome to the Grim Dawn Community League Season 4!", 2);
                GameAPI::SendChatMessage(L"Server", L"Type /help for a listing of chat commands.", 2);
            });
        }
    }
}