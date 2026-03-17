#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <cwctype>
#include <filesystem>
#include "ChatAPI.h"
#include "ChatHandler.h"
#include "GameHandler.h"
#include "StringConvert.h"
#include "URI.h"

const std::unordered_map<std::string, uint32_t> challengeCategoryMap =
{
    { "Level Limited Challenges", 1 },
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

// Prototype since we need to reference the actual command table, which isn't defined until later
bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item);

bool HandleChatGlobalCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    ChatAPI::SetChatPrefix(L"/g ");

    type = ChatAPI::CHAT_TYPE_GLOBAL;

    std::wstring subcommand = message.substr(0, message.find(L" "));
    std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);
    std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), std::towlower);
    std::transform(args.begin(), args.end(), args.begin(), std::towlower);
    if ((subcommand == L"on") && (subcommand.size() == message.size()))
    {
        if (channel == 0)
            channel = 1;

        ChatAPI::SetChatChannel(channel);
        return false;
    }
    else if ((subcommand == L"off") && (subcommand.size() == message.size()))
    {
        ChatAPI::SetChatChannel(0);
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
            if (ChatAPI::SetChatColor(ChatAPI::CHAT_TYPE_GLOBAL, colorCode))
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
        }
        else
        {
            name = L"Server";
            message = L"\"" + args + L"\" is not a valid color. Type \"/h color\" for a list of available color aliases. You can also use a hex code, e.g. #FFFFFF.";
        }
        return true;
    }
    
    if (channel != 0)
    {
        if (channel > ChatAPI::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(ChatAPI::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            spChat->Send("JoinChannel", channel).get();
            return false;
        }
    }

    uint8_t currentChannel = ChatAPI::GetChatChannel();
    if (currentChannel > 0)
    {
        if ((message.empty()) && (item == nullptr))
        {
            name = L"Server";
            message = L"You are currently in global channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            name = CharToWide(spClient->GetUsername());
            spChat->Send("Send", channel, message, std::wstring(), item);
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

// TODO: Delete me
/*bool HandleChatTradeCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    spChatManager->SetChatPrefix(L"/t ");

    type = CHAT_TYPE_TRADE;

    std::wstring subcommand = message.substr(0, message.find(L" "));
    std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);
    std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), std::towlower);
    std::transform(args.begin(), args.end(), args.begin(), std::towlower);
    if ((subcommand == L"on") && (subcommand.size() == message.size()))
    {
        if (channel == 0)
            channel = 1;

        spChatManager->SetChatChannel(CHAT_TYPE_TRADE, channel);
        return false;
    }
    else if ((subcommand == L"off") && (subcommand.size() == message.size()))
    {
        spChatManager->SetChatChannel(CHAT_TYPE_TRADE, 0);
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
            if (spChatManager->SetChatColor(CHAT_TYPE_TRADE, colorCode))
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
        }
        else
        {
            name = L"Server";
            message = L"\"" + args + L"\" is not a valid color. Type \"/h color\" for a list of available color aliases. You can also use a hex code, e.g. #FFFFFF.";
        }
        return true;
    }

    if (channel != 0)
    {
        if (channel > ChatManager::CHAT_CHANNEL_MAX)
        {
            name = L"Server";
            message = L"Invalid channel. The maximum number of channels is " + std::to_wstring(ChatManager::CHAT_CHANNEL_MAX) + L".";
            return true;
        }
        else
        {
            if ((message.empty()) && (item == nullptr))
            {
                spChatManager->SetChatChannel(CHAT_TYPE_TRADE, channel);
            }
            else
            {
                name = CharToWide(spClient->GetUsername());
                spChatManager->SetChannelAndSendMessage(CHAT_TYPE_TRADE, channel, name, message, item);
            }
            return false;
        }
    }

    uint8_t currentChannel = spChatManager->GetChatChannel(CHAT_TYPE_TRADE);
    if (currentChannel > 0)
    {
        if ((message.empty()) && (item == nullptr))
        {
            name = L"Server";
            message = L"You are currently in trade channel " + std::to_wstring(currentChannel) + L".";
        }
        else
        {
            name = CharToWide(spClient->GetUsername());
            spChatManager->SendChatMessage(CHAT_TYPE_TRADE, name, message, item);
            return false;
        }
    }
    else
    {
        name = L"Server";
        message = L"Trade chat is currently disabled. You can enable it by typing /trade ON.";
    }
    return true;
}*/

bool HandleChatOnlineCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    spChat->Send("Online");
    return false;
}

/*pplx::task<web::json::value> GetSeasonChallenges()
{
    Client& client = Client::GetInstance();
    if (client.IsPlayingSeason())
    {
        URI endpoint = client.GetServerGameURL() / "Admin" / "season" / std::to_string(client.GetActiveSeason()->_seasonID) / "challenges";

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        return httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return response.extract_json();
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](pplx::task<web::json::value> task)
        {
            return task.get();
        });
    }

    return pplx::task_from_result(web::json::value::null());
}

pplx::task<std::unordered_set<uint32_t>> GetCompletedChallengeIDs()
{
    Client& client = Client::GetInstance();
    if (client.IsPlayingSeason())
    {
        URI endpoint = client.GetServerGameURL() / "Season" / std::to_string(client.GetActiveSeason()->_seasonID) / "participant-challenges" / std::to_string(client.GetCurrentParticipantID());

        web::http::client::http_client httpClient((utility::string_t)endpoint);
        web::http::http_request request(web::http::methods::GET);

        std::string bearerToken = "Bearer " + client.GetAuthToken();
        request.headers().add(U("Authorization"), bearerToken.c_str());

        return httpClient.request(request).then([](web::http::http_response response)
        {
            if (response.status_code() == web::http::status_codes::OK)
                return response.extract_json();
            else
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        })
        .then([](pplx::task<web::json::value> task)
        {
            std::unordered_set<uint32_t> challengeIDs;
            try
            {
                web::json::array completedChallenges = task.get().as_array();
                for (size_t i = 0; i < completedChallenges.size(); ++i)
                {
                    web::json::value challengeData = completedChallenges[i];
                    uint32_t challengeID = challengeData[U("seasonChallengeId")].as_integer();
                    challengeIDs.insert(challengeID);
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve completed challenges: %s", ex.what());
            }

            return challengeIDs;
        });
    }

    return pplx::task_from_result(std::unordered_set<uint32_t>());
}*/

bool HandleChatChallengesCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    // shuffling this out into a detached thread. On paper this should be worse performance than a task,
    // but it shouldn't matter unless someone spams /challenges literally several times per second.
    // the dependencies are already tasks so it can be turned into a task if REALLY needed, but it would
    // (for this particular function) make the code much more complicated and harder to follow.

    // TODO Use new ChallengeManager and assume data is already filled in - by the time /challenges can be used, this should be the case
    /*
    std::thread challengesInfo([channel]()
    {
        json challenges;
        std::unordered_set<uint32_t> challengeIDs;

        try
        {
            challenges = GetSeasonChallenges().get();
            challengeIDs = GetCompletedChallengeIDs().get();
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season challenge data: %s", ex.what());
        }

        if (!challenges.is_null())
        {
            if (channel == 0)
            {
                std::wstring overview = L"Challenge overview for ";
                overview += CharToWide(spClient->GetUsername());
                overview += L": ";
                GameAPI::SendChatMessage(L"Server", overview, CHAT_TYPE_NORMAL);

                std::unordered_map<uint32_t, uint32_t> challengeCount;
                std::unordered_map<uint32_t, uint32_t> completedCount;
                for (json challenge : challenges)
                {
                    std::string challengeCategory = challenge.at("categoryName").get<std::string>();
                    uint32_t challengeID = challenge.at("seasonChallengeId").get<uint32_t>();

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

                        message += CharToWide(it->first);
                        message += L" (";
                        message += std::to_wstring(completedCount[i]);
                        message += L"/";
                        message += std::to_wstring(challengeCount[i]);
                        message += L")";

                        ChatType chatType = (completedCount[i] == challengeCount[i]) ? CHAT_TYPE_TRADE : CHAT_TYPE_NORMAL;
                        GameAPI::SendChatMessage(L"Server", message, chatType);
                    }
                }
            }
            else
            {
                auto it = std::find_if(challengeCategoryMap.begin(), challengeCategoryMap.end(), [&channel](const std::pair<std::string, uint32_t>& p) { return p.second == channel; });
                if (it != challengeCategoryMap.end())
                {
                    std::wstring message = CharToWide(it->first);
                    message += L" Challenges for ";
                    message += CharToWide(spClient->GetUsername());
                    message += L": ";
                    GameAPI::SendChatMessage(L"Server", message, CHAT_TYPE_NORMAL);
                }
                else
                {
                    std::wstring message = std::to_wstring(channel);
                    message += L" is not a valid challenge category.";
                    GameAPI::SendChatMessage(L"Server", message, CHAT_TYPE_NORMAL);
                    return;
                }

                for (json challenge : challenges)
                {
                    std::wstring challengeName = CharToWide(challenge.at("challengeName").get<std::string>());
                    std::wstring challengeDifficulty = CharToWide(challenge.at("difficulties").get<std::string>());
                    std::string challengeCategory = challenge.at("categoryName").get<std::string>();

                    auto it = challengeCategoryMap.find(challengeCategory);
                    if ((it != challengeCategoryMap.end()) && (it->second == channel))
                    {
                        uint32_t challengeLevel = 0;
                        if (challenge.contains("maxLevel"))
                            challengeLevel = challenge.at("maxLevel").get<uint32_t>();

                        uint32_t challengePoints = challenge.at("pointValue").get<uint32_t>();
                        uint32_t challengeID = challenge.at("seasonChallengeId").get<uint32_t>();

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

                        ChatType chatType = complete ? CHAT_TYPE_TRADE : CHAT_TYPE_NORMAL;
                        GameAPI::SendChatMessage(L"Server", message, chatType);
                    }
                }
            }
        }
    });

    challengesInfo.detach();*/
    return false;
}

bool HandleChatMuteCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (message.size() > 0)
    {
        std::wstring username = message.substr(0, message.find(L" "));
        std::string clientUsername = spClient->GetUsername();

        if (username.size() == clientUsername.size())
        {
            bool matches = true;
            for (size_t i = 0; i < clientUsername.size(); ++i)
            {
                if (std::towlower(username[i]) != std::tolower(clientUsername[i]))
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                GameAPI::SendChatMessage(L"Server", L"You cannot mute yourself.", ChatAPI::CHAT_TYPE_NORMAL);
                return false;
            }
        }

        spChat->Send("Mute", username);
    }
    else
    {
        const std::unordered_set<std::wstring>& mutedList = ChatAPI::GetMutedList();
        if (mutedList.size() == 0)
        {
            GameAPI::SendChatMessage(L"Server", L"You have not muted any players.", ChatAPI::CHAT_TYPE_NORMAL);
        }
        else
        {
            GameAPI::SendChatMessage(L"Server", L"You have muted the following players:", ChatAPI::CHAT_TYPE_NORMAL);
            for (const std::wstring& playerName : mutedList)
            {
                std::wstring message = L"  " + playerName;
                GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);
            }
        }
    }

    return false;
}

bool HandleChatUnmuteCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (message.size() > 0)
    {
        std::wstring username = message.substr(0, message.find(L" "));
        spChat->Send("Unmute", username);
    }
    return false;
}

bool HandleChatWhisperCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    std::wstring username = message.substr(0, message.find(L" "));
    std::wstring text = (username.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);

    if (!text.empty())
    {
        spChat->Send("Send", channel, text, name, item);

        // TODO: Make this work with websockets
        /*pplx::create_task([username, text]()
        {
            Client& client = Client::GetInstance();
            URI endpoint = client.GetServerChatURL() / "chat" / "user" / username / "direct";

            web::json::value requestBody;
            requestBody[U("username")] = JSONString(client.GetUsername());
            requestBody[U("messageBody")] = JSONString(text);
            requestBody[U("type")] = CHAT_TYPE_WHISPER;

            web::http::client::http_client httpClient((utility::string_t)endpoint);
            web::http::http_request request(web::http::methods::POST);
            request.set_body(requestBody);

            std::string bearerToken = "Bearer " + client.GetAuthToken();
            request.headers().add(U("Authorization"), bearerToken.c_str());

            web::http::http_response response = httpClient.request(request).get();
            web::http::status_code status = response.status_code();
            switch (status)
            {
                case web::http::status_codes::OK:
                case web::http::status_codes::BadRequest:
                case web::http::status_codes::InternalError:
                    return status;
                default:
                    throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        })
        .then([username, text](pplx::task<web::http::status_code> task)
        {
            try
            {
                switch (task.get())
                {
                    case web::http::status_codes::OK:
                    {
                        std::wstring user = L"[To " + username + L"]";
                        GameAPI::SendChatMessage(user, text, CHAT_TYPE_NORMAL);
                        spChatManager->SetChatPrefix(L"/w " + username + L" ");
                        break;
                    }
                    case web::http::status_codes::BadRequest:
                    {
                        std::wstring message = username + L" is not currently online.";
                        GameAPI::SendChatMessage(L"Server", message, CHAT_TYPE_NORMAL);
                        break;
                    }
                    case web::http::status_codes::InternalError:
                    {
                        std::wstring message = username + L" was not found on the server.";
                        GameAPI::SendChatMessage(L"Server", message, CHAT_TYPE_NORMAL);
                        break;
                    }
                }
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Failed to send whisper message: %", ex.what());
            }
        });*/
    }

    return false;
}

bool IsBetaBranch()
{
    return (spClient->GetBranch() == SEASON_BRANCH_BETA);
}

bool HandleBetaAddItemCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (IsBetaBranch())
    {
        std::wstring subcommand = message.substr(0, message.find(L" "));
        std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);

        int32_t stackCount = 1;
        try
        {
            stackCount = std::stoi(args);
            if (stackCount <= 0)
                stackCount = 1;
        }
        catch (std::exception&) {}

        GameAPI::ItemReplicaInfo itemInfo;
        itemInfo._itemID = EngineAPI::CreateObjectID();
        itemInfo._itemName = WideToChar(subcommand);
        itemInfo._itemStackCount = stackCount;
        itemInfo._itemSeed = GameAPI::GenerateItemSeed();

        if (void* newItem = GameAPI::CreateItem(itemInfo))
        {
            void* mainPlayer = GameAPI::GetMainPlayer();
            GameAPI::SetItemVisiblePlayer(newItem, EngineAPI::GetObjectID(mainPlayer));
            GameAPI::GiveItemToPlayer(mainPlayer, newItem, true, true);
        }
        return false;
    }
    return true;
}

bool HandleBetaAddMoneyCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (IsBetaBranch())
    {
        std::wstring subcommand = message.substr(0, message.find(L" "));

        int32_t amount = 0;
        try
        {
            amount = std::stoi(subcommand);
        }
        catch (std::exception&) {}

        void* mainPlayer = GameAPI::GetMainPlayer();
        GameAPI::AddOrSubtractMoney(mainPlayer, amount);
        return false;
    }
    return true;
}

bool HandleBetaDumpTagsCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (IsBetaBranch())
    {
        void* mainPlayer = GameAPI::GetMainPlayer();
        if (mainPlayer != nullptr)
        {
            std::wstring playerName = GameAPI::GetPlayerName(mainPlayer);
            std::string filename = "tags_" + WideToChar(playerName) + ".txt";
            std::ofstream out(filename, std::ofstream::out);
            for (auto difficulty : GameAPI::GAME_DIFFICULTIES)
            {
                const std::vector<GameAPI::TriggerToken>& tokens = GameAPI::GetPlayerTokens(mainPlayer, difficulty);
                for (size_t i = 0, index = 0; i < tokens.size(); ++i)
                {
                    std::string tokenString = tokens[i];

                    switch (difficulty)
                    {
                        case GameAPI::GAME_DIFFICULTY_NORMAL:
                            out << "[N] ";
                            break;
                        case GameAPI::GAME_DIFFICULTY_ELITE:
                            out << "[E] ";
                            break;
                        case GameAPI::GAME_DIFFICULTY_ULTIMATE:
                            out << "[U] ";
                            break;
                    }
                    out << tokenString << std::endl;
                }
            }
            out.close();

            GameAPI::SendChatMessage(L"Server", std::wstring(L"Tags successfully written to ") + CharToWide(filename), ChatAPI::CHAT_TYPE_NORMAL);
        }

        return false;
    }
    return true;
}

typedef bool (*ChatCommandInfoFilter)();
typedef bool (*ChatCommandHandler)(std::wstring&, std::wstring&, uint32_t&, uint8_t&, void*);

struct ChatCommandInfo
{
    ChatCommandInfoFilter     _filter;
    std::wstring              _blurb;
    std::wstring              _detail;
};

// TODO: Move these strings into a tags file and get them via Localize()
const std::unordered_map<ChatCommandHandler, ChatCommandInfo> chatCommandInfo = 
{
    { HandleChatChallengesCommand, { nullptr,      L"Displays the list of season challenges.", L"Usage: /c, /challenges[category]\n\nDisplays the user's current challenge progress in the season. If no arguments are specified, displays an overview of all challenge categories.\n\n    [category] - Displays a list of individual challenges for the specified challenge category.\n\n" } },
    { HandleChatGlobalCommand,     { nullptr,      L"Sends a message to global chat.", L"Usage: /g, /global[channel] [on|off|color] ...\n\nSends a message to the current global chat channel. If no arguments are specified, displays the current global chat channel.\n\n    [channel] - Sets or switches the current global chat channel. Valid values are 1-15.\n\n    [on/off] - Enables or disables global chat.\n\n    [color] - Sets the color of global chat to a color alias or a 6-digit hex code. Type \"/h color\" for a list of color aliases.\n\n" } },
    { HandleChatHelpCommand,       { nullptr,      L"Displays available commands and their usage.", L"Usage: /h, /help [command]\n\nDisplays a detailed usage message for a chat command. If no arguments are specified, displays all available chat commands.\n\n    [command] - Specifies the command to display help information on.\n\n" } },
    { HandleChatMuteCommand,       { nullptr,      L"Blocks all incoming messages from a user.", L"Usage: /m, /mute [user]\n\nBlocks all incoming messages from a user. If no arguments are specified, displays the list of users that you have currently muted.\n\n    [user] - Specifies the username to be blocked.\n\n" } },
    { HandleChatOnlineCommand,     { nullptr,      L"Displays the number of online users.", L"Usage: /o, /online\n\nDisplays the number of concurrent online users.\n\n" } },
    { HandleChatUnmuteCommand,     { nullptr,      L"Unblocks all incoming messages from a user.", L"Usage: /u, /unmute <user>\n\nUnblocks a user that was previously blocked, allowing you to see their messages again.\n\n    <user> - Specifies the username to be unblocked.\n\n" } },
    { HandleChatWhisperCommand,    { nullptr,      L"Sends a direct message to a user.", L"Usage: /t, /tell <user> ...\n\nSends a direct message to a user.\n\n    <user> - Specifies the username to send a message to.\n\n" } },
    
    // Beta testing commands
    { HandleBetaAddItemCommand,    { IsBetaBranch, L"Adds an item directly into the user's inventory.", L"Usage: /item <dbr_name> <stack_count>\n\nAdds an item directly into the user's inventory.\n\n    <dbr_name> - The full path of the item DBR to add.\n\n    <stack_count> - The stack count of the item. If not specified, this value will be 1.\n\n" } },
    { HandleBetaAddMoneyCommand,   { IsBetaBranch, L"Adds or removes iron bits from the user's inventory.", L"Usage: /money <amount>\n\nAdds or removes iron bits from the user's inventory.\n\n    <amount> - Specifies the amount of iron bits to add. If this value is negative, the amount will be removed instead.\n\n" } },
    { HandleBetaDumpTagsCommand,   { IsBetaBranch, L"Saves character quest tags.", L"Usage: /tags\n\nSaves all quest tags for the current character to a text file.\n\n" } },
};

const std::unordered_map<std::wstring, ChatCommandHandler> chatCommandHandlers =
{
    { L"help",       HandleChatHelpCommand },
    { L"h",          HandleChatHelpCommand },
    { L"global",     HandleChatGlobalCommand },
    { L"g",          HandleChatGlobalCommand },
    { L"tell",       HandleChatWhisperCommand },
    { L"t",          HandleChatWhisperCommand },
    { L"online",     HandleChatOnlineCommand },
    { L"o",          HandleChatOnlineCommand },
    { L"challenges", HandleChatChallengesCommand },
    { L"c",          HandleChatChallengesCommand },
    { L"mute",       HandleChatMuteCommand },
    { L"m",          HandleChatMuteCommand },
    { L"unmute",     HandleChatUnmuteCommand },
    { L"u",          HandleChatUnmuteCommand },

    // Beta testing commands
    { L"item",       HandleBetaAddItemCommand },
    { L"money",      HandleBetaAddMoneyCommand },
    { L"tags",       HandleBetaDumpTagsCommand },
};

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (message.size() == 0)
    {
        std::map<ChatCommandHandler, std::set<std::wstring>> chatCommandLookup;
        for (const auto& pair : chatCommandHandlers)
            chatCommandLookup[pair.second].insert(pair.first);

        std::set<std::wstring> chatCommandStrings;
        for (const auto& pair : chatCommandLookup)
        {
            size_t count = 0;
            std::wstring commandString = L"    ";
            for (const std::wstring& command : pair.second)
            {
                commandString += L"/" + command;
                if (++count < pair.second.size())
                    commandString += L", ";
            }

            auto it = chatCommandInfo.find(pair.first);
            if (it != chatCommandInfo.end())
            {
                const ChatCommandInfo& info = it->second;
                if ((info._filter == nullptr) || (info._filter()))
                {
                    commandString += L" - " + info._blurb;
                    chatCommandStrings.insert(commandString);
                }
            }
        }

        std::wstring message = L"The following chat commands are available:";
        GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);

        for (const std::wstring& command : chatCommandStrings)
        {
            GameAPI::SendChatMessage(L"Server", command, ChatAPI::CHAT_TYPE_NORMAL);
        }

        message = L"Type /help <command> for more information about a specific chat command.";
        GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);
    }
    else
    {
        std::wstring command = message;
        if (command[0] == '/')
            command = command.substr(1);

        if (chatCommandHandlers.count(command) > 0)
        {
            ChatCommandHandler handler = chatCommandHandlers.at(command);
            ChatCommandInfo info = chatCommandInfo.at(handler);
            GameAPI::SendChatMessage(L"Server", info._detail, ChatAPI::CHAT_TYPE_NORMAL);
        }
        else if ((message == L"color") || (message == L"colour"))
        {
            std::map<uint32_t, std::set<std::wstring>> chatColorLookup;
            for (const auto& pair : chatColorMap)
                chatColorLookup[pair.second.GetColorCode()].insert(pair.first);

            std::set<std::wstring> chatColorStrings;
            for (const auto& pair : chatColorLookup)
            {
                size_t count = 0;
                std::wstring commandString = L"    ";
                for (const std::wstring& command : pair.second)
                {
                    commandString += command;
                    if (++count < pair.second.size())
                        commandString += L", ";
                }
                chatColorStrings.insert(commandString);
            }

            std::wstring message = L"The list of available color aliases are:";
            GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);

            for (const std::wstring& command : chatColorStrings)
            {
                GameAPI::SendChatMessage(L"Server", command, ChatAPI::CHAT_TYPE_NORMAL);
            }
        }
        else
        {
            std::wstring message = L"Command \"" + command + L"\" was not found.";
            GameAPI::SendChatMessage(L"Server", message, ChatAPI::CHAT_TYPE_NORMAL);
        }
    }
    return false;
}

bool ProcessChatCommand(std::wstring& name, std::wstring& message, uint8_t& type, void* item)
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
            if (!handler(name, message, channel, type, item))
                return false;
        }
    }
    return true;
}

void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t itemID)
{
    typedef void (__thiscall* SendChatMessageProto)(void*, const std::wstring&, const std::wstring&, uint8_t, std::vector<uint32_t>, uint32_t);

    SendChatMessageProto callback = (SendChatMessageProto)HookManager::GetOriginalFunction(GAME_DLL, GameAPI::GAPI_NAME_SEND_CHAT_MESSAGE);
    if (callback)
    {
        // The "actual" name and message that are used for the final SendChatMessage() call
        // This may be different from the original values as a result of some of the chat commands
        std::wstring realName = name;
        std::wstring realMessage = message;

        if (spClient->IsPlayingSeason())
        {
            ChatAPI::SetChatPrefix({});

            // If an item is linked, load the saved chat window text from before the window was closed
            if (itemID != 0)
                realMessage = ChatAPI::GetSavedText();

            // If handling an interrupting chat command, return so we don't print the message
            void* item = EngineAPI::FindObjectByID(itemID);
            if (!ProcessChatCommand(realName, realMessage, type, item))
                return;
        }

        callback(_this, realName, realMessage, type, targets, itemID);
    }
}
