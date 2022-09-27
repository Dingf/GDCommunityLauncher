#include "ChatClient.h"
#include "ClientHandlers.h"
#include "Quest.h"

bool HasParticipationTokenFromAPI(PULONG_PTR mainPlayer, const SeasonInfo* seasonInfo)
{
    for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
    {
        const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, difficulty);
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            std::string token = tokens[i].GetTokenString();
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
    std::filesystem::path characterPath = std::filesystem::path(GameAPI::GetBaseFolder()) / "save" / "user" / "_";
    characterPath += playerName;

    if (std::filesystem::is_directory(characterPath))
    {
        for (const auto& it : std::filesystem::recursive_directory_iterator(characterPath))
        {
            Quest questData;
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
                        return true;
                }
            }
        }
    }
    return false;
}

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
            bool hasParticipationToken = GameAPI::HasToken(mainPlayer, seasonInfo->_participationToken) ||
                                         HasParticipationTokenFromAPI(mainPlayer, seasonInfo) || 
                                         HasParticipationTokenFromFile(playerName, seasonInfo);

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