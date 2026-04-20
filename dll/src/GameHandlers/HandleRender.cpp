#include <unordered_map>
#include "GameHandler.h"
#include "ServerHandler.h"
#include "DeathRecap.h"
#include "DungeonDatabase.h"
#include "StringConvert.h"
#include "Version.h"

void BuildLeagueInfoText(std::wstring& message)
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

void HandleRenderStyledText2D(void* _this, EngineAPI::Rect rect, const EngineAPI::Color& color1, const EngineAPI::Color& color2, const wchar_t* text, void* font, int unk1, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int fontStyleFlag, int fontLayout)
{
    typedef void (__thiscall* RenderTextStyled2DProto)(void*, EngineAPI::Rect, const EngineAPI::Color&, const EngineAPI::Color&, const wchar_t*, void*, int, EngineAPI::GraphicsXAlign, EngineAPI::GraphicsYAlign, int, int);

    RenderTextStyled2DProto callback = (RenderTextStyled2DProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    if (callback)
    {
        std::wstring textString(text);
        std::string areaTag = EngineAPI::GetRegionNameTag();
        std::wstring areaName = EngineAPI::UI::Localize(areaTag.c_str());

        // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x >= 0.0f) && (rect._y >= 0.0f) && (rect._x <= 24.0f) && (rect._y <= 24.0f) && (rect._x == rect._y) && (spClient->IsPlayingSeason()))
        {
            if (textString.empty())
                textString += EngineAPI::UI::Localize("tagRDifficultyTitle01");
            BuildLeagueInfoText(textString);

            callback(_this, rect, color1, color2, textString.c_str(), font, unk1, xAlign, yAlign, fontStyleFlag, fontLayout);
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
            callback(_this, rect, color1, color2, textString.c_str(), font, unk1, xAlign, yAlign, fontStyleFlag, fontLayout);
        }
        else
        {
            callback(_this, rect, color1, color2, text, font, unk1, xAlign, yAlign, fontStyleFlag, fontLayout);
        }
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