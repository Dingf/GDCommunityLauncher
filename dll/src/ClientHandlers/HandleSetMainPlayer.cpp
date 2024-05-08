#include "ChatClient.h"
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

        /*if ((client.IsOfflineMode()) && (modName.empty()))
        {
            GameAPI::SendChatMessage(L"Server", L"Welcome to the Grim Dawn Community League!", EngineAPI::UI::CHAT_TYPE_NORMAL);
            GameAPI::SendChatMessage(L"Server", L"You are currently playing in offline mode. Some features such as chat and cloud stash will not work properly.", EngineAPI::UI::CHAT_TYPE_NORMAL);
            GameAPI::SendChatMessage(L"Server", L"You can play online during a season or by becoming a patron at https://www.patreon.com/bePatron?u=46741640.", EngineAPI::UI::CHAT_TYPE_NORMAL);
        }
        else*/
        if ((mainPlayer) && (seasonInfo))
        {
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
            bool hasParticipationToken = GameAPI::PlayerHasToken(mainPlayer, seasonInfo->_participationToken) ||
                                         HasParticipationTokenFromAPI(mainPlayer, seasonInfo) || 
                                         HasParticipationTokenFromFile(playerName, seasonInfo);

            if (hasParticipationToken)
                client.SetActiveCharacter(playerName);

            // Initialize the chat window
            EngineAPI::UI::ChatWindow::GetInstance().Initialize();
            ChatClient::GetInstance().DisplayWelcomeMessage();
        }
    }
}