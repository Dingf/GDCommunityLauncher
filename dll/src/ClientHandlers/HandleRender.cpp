#include "ClientHandlers.h"

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

        // If the player is in-game on the season mod, append the league info to the difficulty text in the upper left corner
        // We modify the text instead of creating new text because that way it preserves the Z-order and doesn't conflict with the loading screen/pause overlay/etc.
        if ((rect._x >= 0.0f) && (rect._y >= 0.0f) && (rect._x <= 24.0f) && (rect._y <= 24.0f) && (rect._x == rect._y) && (client.IsParticipatingInSeason()))
        {
            std::wstring textString(text);
            if (textString.empty())
                textString += L"Normal";
            textString += client.GetLeagueInfoText();

            callback(_this, rect, textString.c_str(), style, unk1, xAlign, yAlign, layout);
        }
        else
        {
            callback(_this, rect, text, style, unk1, xAlign, yAlign, layout);
        }
    }
}