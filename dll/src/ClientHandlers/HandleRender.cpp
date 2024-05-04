#include <unordered_map>
#include "ClientHandler.h"
#include "DungeonDatabase.h"

void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::UI::GraphicsXAlign xAlign, EngineAPI::UI::GraphicsYAlign yAlign, int layout)
{
    typedef void (__thiscall* RenderTextStyled2DProto)(void*, const EngineAPI::Rect&, const wchar_t*, const std::string&, float, EngineAPI::UI::GraphicsXAlign, EngineAPI::UI::GraphicsYAlign, int);

    RenderTextStyled2DProto callback = (RenderTextStyled2DProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    if (callback)
    {
        Client& client = Client::GetInstance();

        std::wstring textString(text);
        std::string areaTag = EngineAPI::GetRegionNameTag();
        std::wstring areaName = EngineAPI::UI::Localize(areaTag.c_str());

        // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x >= 0.0f) && (rect._y >= 0.0f) && (rect._x <= 24.0f) && (rect._y <= 24.0f) && (rect._x == rect._y) && (client.IsPlayingSeason()))
        {
            if (textString.empty())
                textString += L"Normal";
            textString += client.GetLeagueInfoText();

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