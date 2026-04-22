#include <chrono>
#include <regex>
#include <sstream>
#include <unordered_map>
#include "GameHandler.h"
#include "ServerHandler.h"
#include "DeathRecap.h"
#include "DungeonDatabase.h"
#include "StringConvert.h"
#include "Version.h"

static inline void BuildLeagueInfoText(std::wstring& message)
{
    message += L"\n";

    const SeasonInfo* activeSeason = spClient->GetActiveSeason();
    if (spClient->IsOfflineMode())
    {
        message += EngineAPI::UI::Localize("tagGDCLVersion", GDCL_VERSION_WIDE);
        message += L"\n";
    }
    else if (activeSeason)
    {
        message += CharToWide(activeSeason->_displayName);
        message += L"\n";
    }
    message += CharToWide(spClient->GetUsername());

    if (spClient->IsOfflineMode())
    {
        message += EngineAPI::UI::Localize("tagGDCLInfoOffline");
    }
    else if ((spServer->IsConnected()) && (activeSeason))
    {
        if (GameAPI::IsCloudStorageEnabled())
        {
            message += EngineAPI::UI::Localize("tagGDCLInfoCloudSave");
        }
        else if (EngineAPI::IsMultiplayer())
        {
            message += EngineAPI::UI::Localize("tagGDCLInfoMultiplayer");
        }
        else
        {
            uint32_t points = spClient->GetPoints();
            uint32_t rank = spClient->GetRank();
            if ((points > 0) && (rank > 0))
            {
                message += EngineAPI::UI::Localize("tagGDCLInfoRankedPoints", rank, points);
            }
            else
            {
                message += EngineAPI::UI::Localize("tagGDCLInfoPoints", points);
            }
        }
    }
    else
    {
        message += EngineAPI::UI::Localize("tagGDCLInfoDisconnected");
    }
}

static inline std::wstring GetTimeText()
{
    auto now = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}.get_local_time();
    std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::milliseconds>(now - std::chrono::floor<std::chrono::days>(now)));

    int32_t hours = hms.hours().count();
    int32_t minutes = hms.minutes().count();
    int32_t seconds = hms.seconds().count();

    std::wstringstream stream;
    stream << ((hours == 0) ? 12 : (((hours - 1) % 12) + 1)) << " : ";
    stream << std::setw(2) << std::setfill(L'0') << minutes;
    stream << " : ";
    stream << std::setw(2) << std::setfill(L'0') << seconds;
    stream << " ";
    stream << ((hours >= 12) ? "PM" : "AM");

    return stream.str();
}

void HandleRenderStyledText2D(void* _this, EngineAPI::Rect rect, const EngineAPI::Color& color1, const EngineAPI::Color& color2, const wchar_t* text, void* font, int32_t size, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int32_t style, int32_t layout)
{
    typedef void (__thiscall* RenderTextStyled2DProto)(void*, EngineAPI::Rect, const EngineAPI::Color&, const EngineAPI::Color&, const wchar_t*, void*, int, EngineAPI::GraphicsXAlign, EngineAPI::GraphicsYAlign, int, int);

    RenderTextStyled2DProto callback = (RenderTextStyled2DProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    if (callback)
    {
        if (spClient->IsPlayingSeasonOrOffline())
        {
            std::wstring textString(text);
            std::string areaTag = EngineAPI::GetRegionNameTag();
            std::wstring areaName = EngineAPI::UI::Localize(areaTag.c_str());

            static std::wregex timeRegex(L"(\\d+):(\\d{2}):(\\d{2}) (AM|PM)");
            if (std::regex_match(textString, timeRegex))
            {
                // Don't render the time string if playing the season, since it will be in the league info text instead
                return;
            }
            // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
            // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
            else if ((rect._x >= 0.0f) && (rect._y >= 0.0f) && (rect._x <= 24.0f) && (rect._y <= 24.0f) && (rect._x == rect._y))
            {
                if (textString.empty())
                    textString += EngineAPI::UI::Localize("tagRDifficultyTitle01");

                BuildLeagueInfoText(textString);

                // This checks whether "Display Time" is enabled
                // Create a new instance of text because the text gets weirdly indented once color tags are added
                if (EngineAPI::GetBoolOption(65))
                {
                    std::wstring timeText = GetTimeText();
                    callback(_this, { rect._x, rect._y + (size * 4.0f), rect._w, rect._h }, color1, color2, timeText.c_str(), font, size, xAlign, yAlign, style, layout);
                }

                return callback(_this, rect, color1, color2, textString.c_str(), font, size, xAlign, yAlign, style, layout);
            }
            // Display the current level of scaling dungeons
            else if (textString == areaName)
            {
                DungeonDatabase& database = DungeonDatabase::GetInstance();
                if (database.IsDungeonZone(areaTag))
                {
                    const auto& entry = database.GetEntryByZone(areaTag);
                    if (entry._active)
                        textString += EngineAPI::UI::Localize("tagGDCLBoundlessDungeonLevel", entry._level);
                }
                return callback(_this, rect, color1, color2, textString.c_str(), font, size, xAlign, yAlign, style, layout);
            }
        }

        callback(_this, rect, color1, color2, text, font, size, xAlign, yAlign, style, layout);
    }
}

void HandleRender(void* _this)
{
    typedef void (__thiscall* RenderProto)(void*);

    RenderProto callback = (RenderProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER);
    if (callback)
    {
        callback(_this);
        DeathRecap::Update();
    }
}