#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <regex>
#include <cwctype>
#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ChatClient.h"
#include "Character.h"
#include "JSONObject.h"
#include "URI.h"
#include "Log.h"

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    GameAPI::SendChatMessage(L"/g, /global[CHANNEL] [ON/OFF]", L"Sends a message to the current global chat channel. If no arguments are specified, displays the current global chat channel.\n    [CHANNEL] - Switches the current global chat channel.\n    [ON/OFF] - Enables or disables global chat.\n ", 0);
    GameAPI::SendChatMessage(L"/tr, /trade[CHANNEL] [ON/OFF]", L"Sends a message to the current trade chat channel. If no arguments are specified, displays the current trade chat channel.\n    [CHANNEL] - Switches the current trade chat channel.\n    [ON/OFF] - Enables or disables trade chat.\n ", 0);
    GameAPI::SendChatMessage(L"/c, /challenges[CATEGORY]", L"Displays the user's current challenge progress in the season. If no arguments are specified, displays an overview of all challenge categories.\n    [CATEGORY] - Displays challenges for the specified category.\n ", 0);
    GameAPI::SendChatMessage(L"/o, /online", L"Displays the number of current users online.\n ", 0);
    GameAPI::SendChatMessage(L"/h, /help", L"Displays this help message. ", 0);
    return false;
}

const std::unordered_map<std::string, uint32_t> challengeCategoryMap =
{
    { "Leveling Achievements", 1 },
    { "Shattered Realm Clears", 2 },
    { "Skeleton Key Dungeons", 3 },
    { "World Bosses", 4 },
    { "Super Bosses", 5 },
    { "Nemesis Bosses", 6 },
    { "Campaign/Story", 7 },
};

bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/g ");

    type = EngineAPI::UI::CHAT_TYPE_GLOBAL;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
        return false;
    }
    else if (arg == L"off")
    {
        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, 0);
        return false;
    }
    else if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if (message.empty())
            {
                chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                chatClient.SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel, name, message);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in global channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient.SendChatMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, name, message);
            return false;
        }
    }
    else
    {
        name = L"Server";
        message = L"Global chat is currently disabled. You can enable it by typing /global ON.";
    }
    return true;
}

bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    ChatClient& chatClient = ChatClient::GetInstance();
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/tr ");

    type = EngineAPI::UI::CHAT_TYPE_TRADE;

    std::wstring arg = message;
    std::transform(arg.begin(), arg.end(), arg.begin(), std::towlower);
    if (arg == L"on")
    {
        if (channel == 0)
            channel = 1;

        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
        return false;
    }
    else if (arg == L"off")
    {
        chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, 0);
        return false;
    }
    else if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if (message.empty())
            {
                chatClient.SetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                chatClient.SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_TRADE, channel, name, message);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient.GetCurrentChatChannel(EngineAPI::UI::CHAT_TYPE_TRADE);
    if (currentChannel > 0)
    {
        if (message.empty())
        {
            name = L"Server";
            message = L"You are currently in trade channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient.SendChatMessage(EngineAPI::UI::CHAT_TYPE_TRADE, name, message);
            return false;
        }
    }
    else
    {
        name = L"Server";
        message = L"Trade chat is currently disabled. You can enable it by typing /trade ON.";
    }
    return true;
}

bool HandleChatOnlineCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    try
    {
        Client& client = Client::GetInstance();
        URI endpoint = URI(client.GetHostName()) / "chat" / "chat" / "connected-clients";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::array usersArray = responseBody.as_array();

            std::wstring message = L"There are " + std::to_wstring(usersArray.size()) + L" users currently online.";
            GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve online users: %", ex.what());
    }

   return false;
}

web::json::value GetSeasonChallenges()
{
    try
    {
        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            URI endpoint = URI(client.GetHostName()) / "api" / "Admin" / "season" / std::to_string(client.GetActiveSeason()->_seasonID) / "challenges";

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                return response.extract_json().get();
            }
            else
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve challenge list: %", ex.what());
    }

    return web::json::value::null();
}

std::unordered_set<uint32_t> GetCompletedChallengeIDs()
{
    std::unordered_set<uint32_t> challengeIDs;
    try
    {
        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            URI endpoint = URI(client.GetHostName()) / "api" / "Season" / std::to_string(client.GetActiveSeason()->_seasonID) / "participant-challenges" / std::to_string(client.GetParticipantID());

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::GET);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            if (response.status_code() == web::http::status_codes::OK)
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::array completedChallenges = responseBody.as_array();

                for (size_t i = 0; i < completedChallenges.size(); ++i)
                {
                    web::json::value challengeData = completedChallenges[i];
                    uint32_t challengeID = challengeData[U("seasonChallengeId")].as_integer();
                    challengeIDs.insert(challengeID);
                }
            }
            else
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve completed challenges: %", ex.what());
    }
    return challengeIDs;
}

bool HandleChatChallengesCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type)
{
    Client& client = Client::GetInstance();
    web::json::value challenges = GetSeasonChallenges();
    std::unordered_set<uint32_t>& challengeIDs = GetCompletedChallengeIDs();

    if (!challenges.is_null())
    {
        web::json::array challengeList = challenges.as_array();
        if (channel == 0)
        {
            {
                std::wstring message = L"Challenge overview for ";
                message += std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                message += L": ";
                GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
            }

            std::unordered_map<uint32_t, uint32_t> challengeCount;
            std::unordered_map<uint32_t, uint32_t> completedCount;
            for (size_t i = 0; i < challengeList.size(); ++i)
            {
                web::json::value challengeData = challengeList[i];
                std::string challengeCategory = JSONString(challengeData[U("categoryName")].serialize());
                uint32_t challengeID = challengeData[U("seasonChallengeId")].as_integer();

                auto it = challengeCategoryMap.find(challengeCategory);
                if (it != challengeCategoryMap.end())
                {
                    uint32_t categoryNumber = it->second;
                    challengeCount[categoryNumber]++;

                    if (challengeIDs.count(challengeID) > 0)
                        completedCount[categoryNumber]++;
                }
            }

            for (size_t i = 1; i <= challengeCount.size(); ++i)
            {
                auto it = std::find_if(challengeCategoryMap.begin(), challengeCategoryMap.end(), [&i](const std::pair<std::string, uint32_t>& p) { return p.second == i; });
                if (it != challengeCategoryMap.end())
                {
                    std::wstring message = L"    ";
                    message += std::to_wstring(i);
                    message += L" - ";

                    message += std::wstring(it->first.begin(), it->first.end());
                    message += L" (";
                    message += std::to_wstring(completedCount[i]);
                    message += L"/";
                    message += std::to_wstring(challengeCount[i]);
                    message += L")";

                    EngineAPI::UI::ChatType type = (completedCount[i] == challengeCount[i]) ? EngineAPI::UI::CHAT_TYPE_TRADE : EngineAPI::UI::CHAT_TYPE_NORMAL;
                    GameAPI::SendChatMessage(L"Server", message, type);
                }
            }
        }
        else
        {
            auto it = std::find_if(challengeCategoryMap.begin(), challengeCategoryMap.end(), [&channel](const std::pair<std::string, uint32_t>& p) { return p.second == channel; });
            if (it != challengeCategoryMap.end())
            {
                std::wstring message = std::wstring(it->first.begin(), it->first.end());
                message += L" Challenges for ";
                message += std::wstring(client.GetUsername().begin(), client.GetUsername().end());
                message += L": ";
                GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
            }
            else
            {
                std::wstring message = std::to_wstring(channel);
                message += L" is not a valid challenge category.";
                GameAPI::SendChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
                return false;
            }

            for (size_t i = 0; i < challengeList.size(); ++i)
            {
                web::json::value challengeData = challengeList[i];
                std::wstring challengeName = challengeData[U("challengeName")].as_string();
                std::wstring challengeDifficulty = challengeData[U("maxDifficulty")].as_string();
                std::string challengeCategory = JSONString(challengeData[U("categoryName")].serialize());

                auto it = challengeCategoryMap.find(challengeCategory);
                if ((it != challengeCategoryMap.end()) && (it->second == channel))
                {
                    uint32_t challengeLevel = 0;
                    web::json::value maxLevelValue = challengeData[U("maxLevel")];
                    if (!maxLevelValue.is_null())
                        challengeLevel = maxLevelValue.as_integer();
                    uint32_t challengePoints = challengeData[U("pointValue")].as_integer();
                    uint32_t challengeID = challengeData[U("seasonChallengeId")].as_integer();

                    bool complete = (challengeIDs.count(challengeID) > 0);

                    std::wstring message = L"  [";
                    if (complete)
                        message += L"X";
                    else
                        message += L"  ";
                    message += L"]  ";
                    message += challengeName;
                    message += L" ";

                    // Avoid repeating the level/difficulty suffix for challenges which already have the suffix in their name
                    std::wstring suffix = L"(";
                    if (challengeLevel > 0)
                    {
                        suffix += L"Lv";
                        suffix += std::to_wstring(challengeLevel);
                        suffix += L" ";
                    }
                    suffix += challengeDifficulty;
                    suffix += L")";

                    if ((challengeName.size() < suffix.size()) || (challengeName.compare(challengeName.size() - suffix.size(), suffix.size(), suffix) != 0))
                    {
                        message += suffix;
                        message += L" ";
                    }
                    
                    message += L"~ ";
                    message += std::to_wstring(challengePoints);
                    message += L" points";

                    EngineAPI::UI::ChatType type = complete ? EngineAPI::UI::CHAT_TYPE_TRADE : EngineAPI::UI::CHAT_TYPE_NORMAL;
                    GameAPI::SendChatMessage(L"Server", message, type);
                }
            }
        }
    }
    return false;
}

typedef bool(*ChatCommandHandler)(std::wstring&, std::wstring&, uint32_t&, uint8_t&);
const std::unordered_map<std::wstring, ChatCommandHandler> chatCommandLookup =
{
    { L"help",       HandleChatHelpCommand },
    { L"h",          HandleChatHelpCommand },
    { L"global",     HandleChatGlobalCommand },
    { L"g",          HandleChatGlobalCommand },
    { L"trade",      HandleChatTradeCommand },
    { L"tr",         HandleChatTradeCommand },
    { L"o",          HandleChatOnlineCommand },
    { L"online",     HandleChatOnlineCommand },
    { L"c",          HandleChatChallengesCommand },
    { L"challenges", HandleChatChallengesCommand },
};

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t unk1)
{
    typedef void(__thiscall* HandleSendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    HandleSendChatMessageProto callback = (HandleSendChatMessageProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        std::wstring realName = name;
        std::wstring realMessage = message;

        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            chatWindow.SetChatPrefix({});

            std::wsmatch match;
            std::wregex commandRegex(L"^\\/([A-Za-z_]+)(\\d*)\\s*(.*)$");
            if (std::regex_match(message, match, commandRegex))
            {
                std::wstring command = match.str(1);
                std::transform(command.begin(), command.end(), command.begin(), std::towlower);
                uint32_t channel = 0;
                realMessage = match.str(3);

                try
                {
                    channel = std::stoi(match.str(2));
                }
                catch (std::exception&) {}

                if (chatCommandLookup.count(command) > 0)
                {
                    ChatCommandHandler handler = chatCommandLookup.at(command);
                    if (!handler(realName, realMessage, channel, type))
                        return;
                }
            }
        }

        callback(_this, realName, realMessage, type, targets, unk1);
    }
}
