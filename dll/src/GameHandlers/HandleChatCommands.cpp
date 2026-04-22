#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <cwctype>
#include <filesystem>
#include <boost/algorithm/string.hpp>
#include "ChatAPI.h"
#include "ChatHandler.h"
#include "ChallengeManager.h"
#include "GameHandler.h"
#include "URI.h"

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

        spChat->Send("JoinChannel", channel);
        return false;
    }
    else if ((subcommand == L"off") && (subcommand.size() == message.size()))
    {
        // TODO: Need to properly handle the /g off command
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

                name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
                message = EngineAPI::UI::Localize("tagGDCLChatColorSuccess", outputStream.str());
            }
            else
            {
                name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
                message = EngineAPI::UI::Localize("tagGDCLChatColorFailed");
            }
        }
        else
        {
            name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            message = EngineAPI::UI::Localize("tagGDCLChatColorInvalid", args);
        }
        return true;
    }
    
    if (channel != 0)
    {
        if (channel > ChatAPI::CHAT_CHANNEL_MAX)
        {
            name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            message = EngineAPI::UI::Localize("tagGDCLChatChannelInvalid", ChatAPI::CHAT_CHANNEL_MAX);
            return true;
        }
        else
        {
            spChat->Send("JoinChannel", channel);
            return false;
        }
    }

    uint8_t currentChannel = ChatAPI::GetChatChannel();
    if (currentChannel > 0)
    {
        if ((message.empty()) && (item == nullptr))
        {
            return false;
        }
        else
        {
            name = CharToWide(spClient->GetUsername());
            spChat->Send("Send", currentChannel, message, std::wstring(), item);
            return false;
        }
    }
    else
    {
        name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        message = EngineAPI::UI::Localize("tagGDCLChatGlobalDisabled");
    }
    return true;
}

bool HandleChatServerCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    type = ChatAPI::CHAT_TYPE_SYSTEM;

    std::wstring subcommand = message.substr(0, message.find(L" "));
    std::wstring args = (subcommand.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);
    std::transform(subcommand.begin(), subcommand.end(), subcommand.begin(), std::towlower);
    std::transform(args.begin(), args.end(), args.begin(), std::towlower);
    if (subcommand == L"on")
    {
        ChatAPI::SetServerMessagesEnabled(true);
        name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        message = EngineAPI::UI::Localize("tagGDCLChatAnnouncementsEnabled", ChatAPI::CHAT_CHANNEL_MAX);
        return true;
    }
    else if (subcommand == L"off")
    {
        ChatAPI::SetServerMessagesEnabled(false);
        name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        message = EngineAPI::UI::Localize("tagGDCLChatAnnouncementsDisabled", ChatAPI::CHAT_CHANNEL_MAX);
        return true;
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
            if (ChatAPI::SetChatColor(ChatAPI::CHAT_TYPE_SYSTEM, colorCode))
            {
                std::wstringstream outputStream;
                outputStream << std::hex << std::uppercase << std::setfill(L'0') << std::setw(2) << (colorCode & 0x0000FF) << std::setw(2) << ((colorCode & 0x00FF00) >> 8) << std::setw(2) << ((colorCode & 0xFF0000) >> 16);

                name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
                message = EngineAPI::UI::Localize("tagGDCLChatColorSuccess", outputStream.str());
            }
            else
            {
                name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
                message = EngineAPI::UI::Localize("tagGDCLChatColorFailed");
            }
        }
        else
        {
            name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            message = EngineAPI::UI::Localize("tagGDCLChatColorInvalid", args);
        }
        return true;
    }

    if (subcommand.empty())
    {
        return false;
    }
    else
    {
        name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
        message = EngineAPI::UI::Localize("tagGDCLChatServerInvalid", subcommand.c_str());
    }
    return true;
}

bool HandleChatOnlineCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    spChat->Send("Online");
    return false;
}

bool HandleChatChallengesCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    uint32_t seasonID = 0;
    if (const SeasonInfo* seasonInfo = spClient->GetActiveSeason())
        seasonID = seasonInfo->_seasonID;

    const ChallengeManager::ChallengeList* challengeList = spChallengeManager->GetChallengeList(seasonID);
    const std::map<uint32_t, std::string>& challengeCategoryMap = spChallengeManager->GetChallengeCategories();

    if (!challengeList)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Tried to use challenges command when challenge fetch failed/hasn't happened yet");
        return false;
    }

    name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
    if (channel == 0)
    {
        std::wstring overview = EngineAPI::UI::Localize("tagGDCLChatChallenges01", CharToWide(spClient->GetUsername()).c_str());
        GameAPI::SendChatMessage(name, overview, ChatAPI::CHAT_TYPE_SYSTEM);

        std::unordered_map<uint32_t, uint32_t> challengeCount;
        std::unordered_map<uint32_t, uint32_t> completedCount;
        for (const auto& challengeCategory : challengeCategoryMap)
        {
            challengeCount.emplace(challengeCategory.first, 0);
            completedCount.emplace(challengeCategory.first, 0);
        }

        for (const auto& challenge : *challengeList)
        {
            if (challengeCount.count(challenge.second->_category) == 0)
            {
                Logger::LogMessage(LOG_LEVEL_WARN, "Tried to count challenge with unknown category");
                continue;
            }
            ++challengeCount[challenge.second->_category];
            if (challenge.second->_status == ChallengeStatus::CHALLENGE_STATUS_COMPLETE)
            {
                ++completedCount[challenge.second->_category];
            }
        }

        for (const auto& challengeCategory : challengeCategoryMap)
        {
            if (challengeCount[challengeCategory.first] == 0)
            {
                continue;
            }

            std::wstring message = L"    ";
            message += std::to_wstring(challengeCategory.first);
            message += L" - ";

            message += CharToWide(challengeCategory.second);
            message += L" (";
            message += std::to_wstring(completedCount[challengeCategory.first]);
            message += L"/";
            message += std::to_wstring(challengeCount[challengeCategory.first]);
            message += L")";

            ChatAPI::ChatType chatType = (completedCount[challengeCategory.first] == challengeCount[challengeCategory.first]) ? ChatAPI::CHAT_TYPE_SYSTEM : ChatAPI::CHAT_TYPE_NORMAL;
            GameAPI::SendChatMessage(name, message, chatType);
        }
    }
    else
    {
        auto it = std::find_if(challengeCategoryMap.begin(), challengeCategoryMap.end(), [&channel](const std::pair<uint32_t, std::string>& p) { return p.first == channel; });
        if (it != challengeCategoryMap.end())
        {
            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatChallenges02", CharToWide(it->second).c_str(), CharToWide(spClient->GetUsername()).c_str());
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
        }
        else
        {
            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatChallenges03", channel);
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
            return false;
        }

        for (const auto& challenge : *challengeList)
        {
            if (challenge.second->_category != channel || challenge.second->_status == ChallengeStatus::CHALLENGE_STATUS_HIDDEN)
            {
                continue;
            }

            std::wstring message = L"  [";

            std::wstring challengeName = CharToWide(challenge.second->_name);
            bool completed = challenge.second->_status == ChallengeStatus::CHALLENGE_STATUS_COMPLETE;
            
            if (completed)
                message += L"X";
            else
                message += L"  ";
            message += L"]  ";
            message += challengeName;
            message += L" ";

            // Avoid repeating the level/difficulty suffix for challenges which already have the suffix in their name
            std::wstring suffix = L"(";
            if (challenge.second->_maxLevel > 0)
            {
                suffix += L"Lv";
                suffix += std::to_wstring(challenge.second->_maxLevel);
                suffix += L" ";
            }

            suffix += CharToWide(challenge.second->_difficultyRaw);
            suffix += L")";

            if ((challengeName.size() < suffix.size()) || (challengeName.compare(challengeName.size() - suffix.size(), suffix.size(), suffix) != 0))
            {
                message += suffix;
                message += L" ";
            }

            message += EngineAPI::UI::Localize("tagGDCLChatChallengePoints", challenge.second->_points);

            ChatAPI::ChatType chatType = completed ? ChatAPI::CHAT_TYPE_SYSTEM : ChatAPI::CHAT_TYPE_NORMAL;
            GameAPI::SendChatMessage(name, message, chatType);
        }
    }
    return false;
}

bool HandleChatMuteCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (message.size() > 0)
    {
        std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
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
                GameAPI::SendChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatMute04"), ChatAPI::CHAT_TYPE_SYSTEM);
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
            GameAPI::SendChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatMute03"), ChatAPI::CHAT_TYPE_SYSTEM);
        }
        else
        {
            GameAPI::SendChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatMute02"), ChatAPI::CHAT_TYPE_SYSTEM);
            for (const std::wstring& playerName : mutedList)
            {
                std::wstring message = L"  " + playerName;
                GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
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
        if (ChatAPI::IsPlayerMuted(username))
        {
            spChat->Send("Unmute", username);
        }
        else
        {
            std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            GameAPI::SendChatMessage(name, EngineAPI::UI::Localize("tagGDCLChatUnmute02"), ChatAPI::CHAT_TYPE_SYSTEM);
        }
    }
    return false;
}

bool HandleChatWhisperCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    std::wstring username = message.substr(0, message.find(L" "));
    std::wstring text = (username.size() == message.size()) ? L"" : message.substr(message.find(L" ") + 1);

    if (!text.empty())
        spChat->Send("Send", 0, text, username, item);

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

        ItemReplicaInfo itemInfo;
        itemInfo._itemID = EngineAPI::CreateObjectID();
        itemInfo._name = WideToChar(subcommand);
        itemInfo._stackCount = stackCount;
        itemInfo._seed = GameAPI::GenerateItemSeed();

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

bool HandleBetaLevelUpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    if (IsBetaBranch())
    {
        std::wstring subcommand = message.substr(0, message.find(L" "));

        int32_t amount = 1;
        try { amount = std::stoi(subcommand); }
        catch (std::exception&) {}

        if (amount < 0)
            amount = 0;

        if (void* mainPlayer = GameAPI::GetMainPlayer())
        {
            for (uint32_t i = 0; i < amount; ++i)
            {
                if (GameAPI::IsMaxLevel(mainPlayer))
                    break;
                else
                    GameAPI::IncrementPlayerLevel(mainPlayer);
            }
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
        try { amount = std::stoi(subcommand); }
        catch (std::exception&) {}

        if (void* mainPlayer = GameAPI::GetMainPlayer())
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

            std::wstring name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatSavedTags", filename);
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
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
    std::string              _blurb;
    std::string              _detail;
};

// TODO: Move these strings into a tags file and get them via Localize()
const std::unordered_map<ChatCommandHandler, ChatCommandInfo> chatCommandInfo = 
{
    { HandleChatHelpCommand,       { nullptr,      "tagGDCLCommand01", "tagGDCLCommand01Desc" } },
    { HandleChatGlobalCommand,     { nullptr,      "tagGDCLCommand02", "tagGDCLCommand02Desc" } },
    { HandleChatOnlineCommand,     { nullptr,      "tagGDCLCommand03", "tagGDCLCommand03Desc" } },
    { HandleChatChallengesCommand, { nullptr,      "tagGDCLCommand04", "tagGDCLCommand04Desc" } },
    { HandleChatMuteCommand,       { nullptr,      "tagGDCLCommand05", "tagGDCLCommand05Desc" } },
    { HandleChatUnmuteCommand,     { nullptr,      "tagGDCLCommand06", "tagGDCLCommand06Desc" } },
    { HandleChatWhisperCommand,    { nullptr,      "tagGDCLCommand07", "tagGDCLCommand07Desc" } },
    { HandleChatServerCommand,     { nullptr,      "tagGDCLCommand08", "tagGDCLCommand08Desc" } },
    
    // Beta testing commands
    { HandleBetaAddItemCommand,    { IsBetaBranch, "tagGDCLCommandBeta01", "tagGDCLCommandBeta01Desc" } },
    { HandleBetaLevelUpCommand,    { IsBetaBranch, "tagGDCLCommandBeta02", "tagGDCLCommandBeta02Desc" } },
    { HandleBetaAddMoneyCommand,   { IsBetaBranch, "tagGDCLCommandBeta03", "tagGDCLCommandBeta03Desc" } },
    { HandleBetaDumpTagsCommand,   { IsBetaBranch, "tagGDCLCommandBeta04", "tagGDCLCommandBeta04Desc" } },
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
    { L"s",          HandleChatServerCommand },
    { L"server",     HandleChatServerCommand },

    // Beta testing commands
    { L"item",       HandleBetaAddItemCommand },
    { L"level",      HandleBetaLevelUpCommand },
    { L"money",      HandleBetaAddMoneyCommand },
    { L"tags",       HandleBetaDumpTagsCommand },
};

bool HandleChatHelpCommand(std::wstring& name, std::wstring& message, uint32_t& channel, uint8_t& type, void* item)
{
    name = EngineAPI::UI::Localize("tagGDCLChatDefaultName");
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
                    commandString += L" - ";
                    commandString += EngineAPI::UI::Localize(info._blurb.c_str());
                    chatCommandStrings.insert(commandString);
                }
            }
        }

        std::wstring message = EngineAPI::UI::Localize("tagGDCLChatHelp01");
        GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);

        for (const std::wstring& command : chatCommandStrings)
        {
            GameAPI::SendChatMessage(name, command, ChatAPI::CHAT_TYPE_SYSTEM);
        }

        message = EngineAPI::UI::Localize("tagGDCLChatHelp02");
        GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
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

            message = EngineAPI::UI::Localize(info._detail.c_str());
            boost::replace_all(message, L"^n", L"\n");

            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
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

            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatHelp03");
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);

            for (const std::wstring& command : chatColorStrings)
            {
                GameAPI::SendChatMessage(name, command, ChatAPI::CHAT_TYPE_SYSTEM);
            }
        }
        else
        {
            std::wstring message = EngineAPI::UI::Localize("tagGDCLChatHelpError", command);
            GameAPI::SendChatMessage(name, message, ChatAPI::CHAT_TYPE_SYSTEM);
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

        try { channel = std::stoi(match.str(2)); }
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

            void* item = nullptr;
            // If an item is linked, load the saved chat window text from before the window was closed
            if (itemID != 0)
            {
                realMessage = ChatAPI::GetSavedText();
                item = EngineAPI::FindObjectByID(itemID);
            }

            // If handling an interrupting chat command, return so we don't print the message
            if (!ProcessChatCommand(realName, realMessage, type, item))
                return;
        }

        callback(_this, realName, realMessage, type, targets, itemID);
    }
}
