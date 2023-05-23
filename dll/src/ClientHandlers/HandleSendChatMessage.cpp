#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <cwctype>
#include <filesystem>
#include <cpprest/http_client.h>
#include "ClientHandlers.h"
#include "ChatClient.h"
#include "Character.h"
#include "JSONObject.h"
#include "URI.h"

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

const std::unordered_map<std::wstring, EngineAPI::Color> chatColorMap =
{
    { L"a", EngineAPI::Color::AQUA },       { L"aqua", EngineAPI::Color::AQUA },
    { L"b", EngineAPI::Color::BLUE },       { L"blue", EngineAPI::Color::BLUE },
    { L"c", EngineAPI::Color::CYAN },       { L"cyan", EngineAPI::Color::CYAN },
    { L"d", EngineAPI::Color::DARK_GRAY },  { L"darkgray", EngineAPI::Color::DARK_GRAY }, { L"darkgrey", EngineAPI::Color::DARK_GRAY },
    { L"e", EngineAPI::Color::OLIVE },      { L"olive", EngineAPI::Color::OLIVE },
    { L"f", EngineAPI::Color::FUSHIA },     { L"fushia", EngineAPI::Color::FUSHIA },
    { L"g", EngineAPI::Color::GREEN },      { L"green", EngineAPI::Color::GREEN },
    { L"h", EngineAPI::Color::KHAKI },      { L"khaki", EngineAPI::Color::KHAKI },
    { L"i", EngineAPI::Color::INDIGO },     { L"indigo", EngineAPI::Color::INDIGO },
    { L"j", EngineAPI::Color::SALMON },     { L"salmon", EngineAPI::Color::SALMON },
    { L"k", EngineAPI::Color::BLACK },      { L"black", EngineAPI::Color::BLACK },
    { L"l", EngineAPI::Color::LIME },       { L"lime", EngineAPI::Color::LIME },
    { L"m", EngineAPI::Color::MAROON },     { L"maroon", EngineAPI::Color::MAROON },
    { L"n", EngineAPI::Color::NAVY },       { L"navy", EngineAPI::Color::NAVY },
    { L"o", EngineAPI::Color::ORANGE },     { L"orange", EngineAPI::Color::ORANGE },
    { L"p", EngineAPI::Color::PURPLE },     { L"purple", EngineAPI::Color::PURPLE },
    { L"q", EngineAPI::Color::PINK },       { L"pink", EngineAPI::Color::PINK },
    { L"r", EngineAPI::Color::RED },        { L"red", EngineAPI::Color::RED },
    { L"s", EngineAPI::Color::SILVER },     { L"silver", EngineAPI::Color::SILVER },
    { L"t", EngineAPI::Color::TEAL },       { L"teal", EngineAPI::Color::TEAL },
    { L"u", EngineAPI::Color::CORNFLOWER }, { L"cornflower", EngineAPI::Color::CORNFLOWER },
    { L"v", EngineAPI::Color::VIOLET },     { L"violet", EngineAPI::Color::VIOLET },
    { L"w", EngineAPI::Color::WHITE },      { L"white", EngineAPI::Color::WHITE },
    { L"x", EngineAPI::Color::GRAY },       { L"gray", EngineAPI::Color::GRAY }, { L"grey", EngineAPI::Color::GRAY },
    { L"y", EngineAPI::Color::YELLOW },     { L"yellow", EngineAPI::Color::YELLOW },
    { L"z", EngineAPI::Color::SLATE },      { L"slate", EngineAPI::Color::SLATE },
    { L"gorstak", EngineAPI::Color(0.871f, 0.680f, 1.000f, 1.000f) }
};

bool HandleChatHelpCommand(ChatClient* chatClient, std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    GameAPI::SendChatMessage(L"/g, /global[CHANNEL] [ON/OFF] [COLOR]", L"Sends a message to the current global chat channel. If no arguments are specified, displays the current global chat channel.\n    [CHANNEL] - Switches the current global chat channel.\n    [ON/OFF] - Enables or disables global chat.\n    [COLOR] - Sets the color of global chat to a color alias or a 6-digit hex code.\n ", 0);
    GameAPI::SendChatMessage(L"/t, /trade[CHANNEL] [ON/OFF] [COLOR]", L"Sends a message to the current trade chat channel. If no arguments are specified, displays the current trade chat channel.\n    [CHANNEL] - Switches the current trade chat channel.\n    [ON/OFF] - Enables or disables trade chat.\n    [COLOR] - Sets the color of trade chat to a color alias or a 6-digit hex code.\n ", 0);
    GameAPI::SendChatMessage(L"/c, /challenges[CATEGORY]", L"Displays the user's current challenge progress in the season. If no arguments are specified, displays an overview of all challenge categories.\n    [CATEGORY] - Displays challenges for the specified category.\n ", 0);
    GameAPI::SendChatMessage(L"/o, /online", L"Displays the number of current users online.\n ", 0);
    GameAPI::SendChatMessage(L"/h, /help", L"Displays this help message. ", 0);
    return false;
}

bool HandleChatGlobalCommand(ChatClient* chatClient, std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/g ");

    type = EngineAPI::UI::CHAT_TYPE_GLOBAL;

    std::wstring subcommand = message.substr(0, message.find(L" "));
    std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);
    std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), std::towlower);
    std::transform(args.begin(), args.end(), args.begin(), std::towlower);
    if ((subcommand == L"on") && (subcommand.size() == message.size()))
    {
        if (channel == 0)
            channel = 1;

        chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
        return false;
    }
    else if ((subcommand == L"off") && (subcommand.size() == message.size()))
    {
        chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, 0);
        return false;
    }
    else if ((subcommand == L"color") || (subcommand == L"colour"))
    {
        std::wsmatch match;
        std::wregex colorRegex(L"^#?([A-Fa-f0-9]{6})$");

        uint32_t colorCode = 0;

        auto it = chatColorMap.find(args);
        if (it != chatColorMap.end())
        {
            EngineAPI::Color color = it->second;
            colorCode |= (uint32_t)(color._r * 255);
            colorCode |= ((uint32_t)(color._g * 255) << 8);
            colorCode |= ((uint32_t)(color._b * 255) << 16);
            colorCode |= ((uint32_t)(color._a * 255) << 24);
        }
        else if (std::regex_match(args, match, colorRegex))
        {
            std::wstringstream inputStream;
            inputStream << std::hex << match.str(1);
            inputStream >> colorCode;

            // Swap red and blue values to match color code format
            colorCode = (colorCode & 0x00FFFFFF) | ((colorCode & 0x000000FF) << 24);
            colorCode = (colorCode & 0xFFFFFF00) | ((colorCode & 0x00FF0000) >> 16);
            colorCode = (colorCode & 0xFF00FFFF) | ((colorCode & 0xFF000000) >> 8);
            colorCode = (colorCode & 0x00FFFFFF);
        }

        if (colorCode != 0)
        {
            if (chatWindow.SetChatColor(EngineAPI::UI::CHAT_TYPE_GLOBAL, colorCode))
            {
                std::wstringstream outputStream;
                outputStream << std::hex << std::uppercase << std::setfill(L'0') << std::setw(2) << (colorCode & 0x0000FF) << std::setw(2) << ((colorCode & 0x00FF00) >> 8) << std::setw(2) << ((colorCode & 0xFF0000) >> 16);

                name = L"Server";
                message = L"Changed global chat text color to #" + outputStream.str() + L".";
            }
            else
            {
                name = L"Server";
                message = L"Could not change global chat text color.";
            }
            return true;
        }
    }
    
    if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if ((message.empty()) && (item == nullptr))
            {
                chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());

                chatClient->SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, channel, name, message, item);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient->GetChannel(EngineAPI::UI::CHAT_TYPE_GLOBAL);
    if (currentChannel > 0)
    {
        if ((message.empty()) && (item == nullptr))
        {
            name = L"Server";
            message = L"You are currently in global channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient->SendChatMessage(EngineAPI::UI::CHAT_TYPE_GLOBAL, name, message, item);
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

bool HandleChatTradeCommand(ChatClient* chatClient, std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
    chatWindow.SetChatPrefix(L"/t ");

    type = EngineAPI::UI::CHAT_TYPE_TRADE;

    std::wstring subcommand = message.substr(0, message.find(L" "));
    std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);
    std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), std::towlower);
    std::transform(args.begin(), args.end(), args.begin(), std::towlower);
    if ((subcommand == L"on") && (subcommand.size() == message.size()))
    {
        if (channel == 0)
            channel = 1;

        chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
        return false;
    }
    else if ((subcommand == L"off") && (subcommand.size() == message.size()))
    {
        chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_TRADE, 0);
        return false;
    }
    else if ((subcommand == L"color") || (subcommand == L"colour"))
    {
        std::wsmatch match;
        std::wregex colorRegex(L"^#?([A-Fa-f0-9]{6})$");

        uint32_t colorCode = 0;

        auto it = chatColorMap.find(args);
        if (it != chatColorMap.end())
        {
            EngineAPI::Color color = it->second;
            colorCode |= (uint32_t)(color._r * 255);
            colorCode |= ((uint32_t)(color._g * 255) << 8);
            colorCode |= ((uint32_t)(color._b * 255) << 16);
            colorCode |= ((uint32_t)(color._a * 255) << 24);
        }
        else if (std::regex_match(args, match, colorRegex))
        {
            std::wstringstream inputStream;
            inputStream << std::hex << match.str(1);
            inputStream >> colorCode;

            // Swap red and blue values to match color code format
            colorCode = (colorCode & 0x00FFFFFF) | ((colorCode & 0x000000FF) << 24);
            colorCode = (colorCode & 0xFFFFFF00) | ((colorCode & 0x00FF0000) >> 16);
            colorCode = (colorCode & 0xFF00FFFF) | ((colorCode & 0xFF000000) >> 8);
            colorCode = (colorCode & 0x00FFFFFF);
        }

        if (colorCode != 0)
        {
            if (chatWindow.SetChatColor(EngineAPI::UI::CHAT_TYPE_TRADE, colorCode))
            {
                std::wstringstream outputStream;
                outputStream << std::hex << std::uppercase << std::setfill(L'0') << std::setw(2) << (colorCode & 0x0000FF) << std::setw(2) << ((colorCode & 0x00FF00) >> 8) << std::setw(2) << ((colorCode & 0xFF0000) >> 16);

                name = L"Server";
                message = L"Changed trade chat text color to #" + outputStream.str() + L".";
            }
            else
            {
                name = L"Server";
                message = L"Could not change trade chat text color.";
            }
            return true;
        }
    }

    if (channel != 0)
    {
        if (channel > EngineAPI::UI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(EngineAPI::UI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if ((message.empty()) && (item == nullptr))
            {
                chatClient->SetChannel(EngineAPI::UI::CHAT_TYPE_TRADE, channel);
            }
            else
            {
                Client& client = Client::GetInstance();
                name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());

                chatClient->SetChannelAndSendMessage(EngineAPI::UI::CHAT_TYPE_TRADE, channel, name, message, item);
            }
            return false;
        }
    }

    uint8_t currentChannel = chatClient->GetChannel(EngineAPI::UI::CHAT_TYPE_TRADE);
    if (currentChannel > 0)
    {
        if ((message.empty()) && (item == nullptr))
        {
            name = L"Server";
            message = L"You are currently in trade channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            Client& client = Client::GetInstance();
            name = std::wstring(client.GetUsername().begin(), client.GetUsername().end());
            chatClient->SendChatMessage(EngineAPI::UI::CHAT_TYPE_TRADE, name, message, item);
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

bool HandleChatOnlineCommand(ChatClient* chatClient, std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    pplx::create_task([]()
    {
        try
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerChatURL() / "chat" / "connected-clients";

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
    });

   return false;
}

web::json::value GetSeasonChallenges()
{
    try
    {
        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            URI endpoint = client.GetServerGameURL() / "Admin" / "season" / std::to_string(client.GetActiveSeason()->_seasonID) / "challenges";

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
            URI endpoint = client.GetServerGameURL() / "Season" / std::to_string(client.GetActiveSeason()->_seasonID) / "participant-challenges" / std::to_string(client.GetParticipantID());

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

bool HandleChatChallengesCommand(ChatClient* chatClient, std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    pplx::create_task([channel]()
    {
        Client& client = Client::GetInstance();
        EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();

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

                        EngineAPI::UI::ChatType chatType = (completedCount[i] == challengeCount[i]) ? EngineAPI::UI::CHAT_TYPE_TRADE : EngineAPI::UI::CHAT_TYPE_NORMAL;
                        GameAPI::SendChatMessage(L"Server", message, chatType);
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
                    return;
                }

                for (size_t i = 0; i < challengeList.size(); ++i)
                {
                    web::json::value challengeData = challengeList[i];
                    std::wstring challengeName = challengeData[U("challengeName")].as_string();
                    std::wstring challengeDifficulty = challengeData[U("difficulties")].as_string();
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

                        EngineAPI::UI::ChatType chatType = complete ? EngineAPI::UI::CHAT_TYPE_TRADE : EngineAPI::UI::CHAT_TYPE_NORMAL;
                        GameAPI::SendChatMessage(L"Server", message, chatType);
                    }
                }
            }
        }
    });

    return false;
}

typedef bool (*ChatCommandHandler)(ChatClient*, std::wstring&, std::wstring&, uint32_t&, uint8_t&, void*);
const std::unordered_map<std::wstring, ChatCommandHandler> chatCommandHandlers =
{
    { L"help",       HandleChatHelpCommand },
    { L"h",          HandleChatHelpCommand },
    { L"global",     HandleChatGlobalCommand },
    { L"g",          HandleChatGlobalCommand },
    { L"trade",      HandleChatTradeCommand },
    { L"t",          HandleChatTradeCommand },
    { L"online",     HandleChatOnlineCommand },
    { L"o",          HandleChatOnlineCommand },
    { L"challenges", HandleChatChallengesCommand },
    { L"c",          HandleChatChallengesCommand },
};

bool ChatClient::ProcessChatCommand(std::wstring& name, std::wstring& message, uint8_t& type, void* item)
{
    std::wsmatch match;
    std::wregex commandRegex(L"^\\/([A-Za-z_]+)(\\d*)\\s*(.*)$");
    std::wstring originalMessage = message;
    if (std::regex_match(originalMessage, match, commandRegex))
    {
        std::wstring command = match.str(1);
        std::transform(command.begin(), command.end(), command.begin(), std::towlower);
        uint32_t channel = 0;
        message = match.str(3);

        try
        {
            channel = std::stoi(match.str(2));
        }
        catch (std::exception&) {}

        if (chatCommandHandlers.count(command) > 0)
        {
            ChatCommandHandler handler = chatCommandHandlers.at(command);
            if (!handler(this, name, message, channel, type, item))
                return false;
        }
    }
    return true;
}

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t itemID)
{
    typedef void (__thiscall* SendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    SendChatMessageProto callback = (SendChatMessageProto)HookManager::GetOriginalFunction("Game.dll", GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        // The "actual" name and message that are used for the final SendChatMessage() call
        // This may be different from the original values as a result of some of the chat commands
        std::wstring realName = name;
        std::wstring realMessage = message;

        Client& client = Client::GetInstance();
        if (client.IsParticipatingInSeason())
        {
            EngineAPI::UI::ChatWindow& chatWindow = EngineAPI::UI::ChatWindow::GetInstance();
            chatWindow.SetChatPrefix({});

            // If an item is linked, load the saved chat window text from before the window was closed
            if (itemID != 0)
                realMessage = chatWindow.GetSavedText();

            // If handling an interrupting chat command, return so we don't print the message
            ChatClient& chatClient = ChatClient::GetInstance();
            void* item = EngineAPI::FindObjectByID(itemID);
            if (!chatClient.ProcessChatCommand(realName, realMessage, type, item))
                return;
        }

        callback(_this, realName, realMessage, type, targets, itemID);
    }
}
