#include <unordered_map>
#include "ClientHandlers.h"

std::unordered_map<std::string, std::string> dungeonAreaLookup =
{
    { "tagUGBrokenHills02C", "steps_of_torment" },
    { "tagUGBrokenHills02D", "steps_of_torment" },
    { "tagUGBrokenHills02E", "steps_of_torment" },
    { "tagUGBrokenHills02F", "steps_of_torment" },
    { "tagMapPortValburyDistrict01", "port_valbury" },
    { "tagMapPortValburyDistrict02", "port_valbury" },
    { "tagMapPortValburyCellar", "port_valbury" },
    { "tagMapPortValburyManor", "port_valbury" },
    { "tagMapPortValburyGauntlet", "port_valbury" },
    { "tagMapPortValburyAldritchCellar", "port_valbury" },
    { "tagUGVoidlands01A", "bastion_of_chaos" },
    { "tagUGVoidlands01B", "bastion_of_chaos" },
    { "tagUGVoidlands01C", "bastion_of_chaos" },
    { "tagUGVoidlands01D", "bastion_of_chaos" },
    { "tagGDX1UGAncientGrove01A", "ancient_grove" },
    { "tagGDX1UGAncientGrove01B", "ancient_grove" },
    { "tagGDX1UGAncientGrove01C", "ancient_grove" },
    { "tagGDX1UGAncientGrove01D", "ancient_grove" },
    { "tagGDX2Roguelike_01A", "tomb_of_the_heretic" },
    { "tagGDX2Roguelike_01B", "tomb_of_the_heretic" },
    { "tagGDX2Roguelike_01C", "tomb_of_the_heretic" },
    { "tagGDX2Roguelike_01D", "tomb_of_the_heretic" },
};

void HandleRender(void* _this)
{
    typedef void(__thiscall* RenderProto)(void*);

    RenderProto callback = (RenderProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER);
    if (callback)
    {
        callback(_this);

        // Terminate the process immediately if GI is detected
        if (GetModuleHandle(TEXT("GrimInternalsDll64.dll")))
        {
            ExitProcess(EXIT_SUCCESS);
        }
    }
}

void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int layout)
{
    typedef void(__thiscall* RenderTextStyled2DProto)(void*, const EngineAPI::Rect&, const wchar_t*, const std::string&, float, EngineAPI::GraphicsXAlign, EngineAPI::GraphicsYAlign, int);

    RenderTextStyled2DProto callback = (RenderTextStyled2DProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_RENDER_STYLED_TEXT_2D);
    if (callback)
    {
        Client& client = Client::GetInstance();

        std::wstring textString(text);
        const char* areaTag = EngineAPI::GetAreaNameTag();
        std::wstring areaName = EngineAPI::Localize(areaTag);

        // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x >= 0.0f) && (rect._y >= 0.0f) && (rect._x <= 24.0f) && (rect._y <= 24.0f) && (rect._x == rect._y) && (client.IsParticipatingInSeason()))
        {
            if (textString.empty())
                textString += L"Normal";
            textString += client.GetLeagueInfoText();

            callback(_this, rect, textString.c_str(), style, unk1, xAlign, yAlign, layout);
        }
        // Display the current level of scaling dungeons
        else if (textString == areaName)
        {
            if (dungeonAreaLookup.count(areaTag) > 0)
            {
                const auto& dungeonData = client.GetDungeonData();
                std::string key = dungeonAreaLookup[areaTag];

                if ((dungeonData.count(key) > 0) && (dungeonData.at(key)._active))
                {
                    uint32_t level = dungeonData.at(key)._level;
                    textString += L" (Lv";
                    textString += std::to_wstring(level);
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