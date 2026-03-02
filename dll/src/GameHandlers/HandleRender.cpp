#include <unordered_map>
#include "GameHandler.h"
#include "DeathRecap.h"
#include "DungeonDatabase.h"
#include "StringConvert.h"
#include "Version.h"

void BuildLeagueInfoText(std::wstring& message)
{
    const SeasonInfo* activeSeason = spClient->GetActiveSeason();
    if (spClient->IsOfflineMode())
    {
        std::string versionText = GDCL_VERSION;
        message += L"\n";
        message += L"GDCL v";
        message += CharToWide(versionText);
    }
    else if (activeSeason)
    {
        message += L"\n";
        message += CharToWide(activeSeason->_displayName);
    }
    message += L"\n";
    message += CharToWide(spClient->GetUsername());

    if (spClient->IsOfflineMode())
    {
        message += L" {^L}(Offline Mode)";
    }
    // TODO: The online check needs to be for the websocket connection to the server (spServerSocket)
    else if (/*(_online) && */(activeSeason))
    {
        if (GameAPI::IsCloudStorageEnabled())
        {
            message += L" {^Y}(Disable Cloud Saving)";
        }
        else if (EngineAPI::IsMultiplayer())
        {
            message += L" {^Y}(Multiplayer)";
        }
        else
        {
            uint32_t points = spClient->GetPoints();
            uint32_t rank = spClient->GetRank();
            if ((points > 0) && (rank > 0))
            {
                message += L" {^L}(Rank ";
                message += std::to_wstring(rank);
                message += L" ~ ";
            }
            else
            {
                message += L" {^L}(";
            }
            message += std::to_wstring(points);
            message += L" points)";
        }
    }
    else
    {
        message += L" {^R}(Disconnected)";
    }
}


void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int layout)
{
    typedef void (__thiscall* RenderTextStyled2DProto)(void*, const EngineAPI::Rect&, const wchar_t*, const std::string&, float, EngineAPI::GraphicsXAlign, EngineAPI::GraphicsYAlign, int);

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
                textString += L"Normal";
            BuildLeagueInfoText(textString);

            callback(_this, rect, textString.c_str(), style, unk1, xAlign, yAlign, layout);
        }
        // Display the current level of scaling dungeons
        else if (textString == areaName)
        {
            DungeonDatabase& database = DungeonDatabase::GetInstance();
            if (database.IsDungeonZone(areaTag))
            {
                const auto& entry = database.GetEntryByZone(areaTag);
                if (entry._active)
                {
                    textString += L"{^O} (Lv";
                    textString += std::to_wstring(entry._level);
                    textString += L")";
                }
            }
            callback(_this, rect, textString.c_str(), style, unk1, xAlign, yAlign, layout);
        }
        else
        {
            callback(_this, rect, text, style, unk1, xAlign, yAlign, layout);
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