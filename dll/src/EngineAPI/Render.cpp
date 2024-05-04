#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{
    
void* GetGraphicsEngine()
{
    typedef void* (__thiscall* GetGraphicsEngineProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetGraphicsEngineProto callback = (GetGraphicsEngineProto)GetProcAddress(engineDLL, EAPI_NAME_GET_GRAPHICS_ENGINE);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback(*engine);
}

void* GetCanvas()
{
    typedef void* (__thiscall* GetCanvasProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetCanvasProto callback = (GetCanvasProto)GetProcAddress(engineDLL, EAPI_NAME_GET_CANVAS);
    void* graphicsEngine = GetGraphicsEngine();

    if ((!callback) || (!graphicsEngine))
        return nullptr;

    return callback(graphicsEngine);
}

void RenderText2D(int x, int y, const Color& color, const wchar_t* text, void* font, int size, UI::GraphicsXAlign xAlign, UI::GraphicsYAlign yAlign, int style, int layout)
{
    typedef void (__thiscall* RenderText2DProto)(void*, int, int, const Color&, const void*, void*, int, int, int, int, int);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    RenderText2DProto callback = (RenderText2DProto)GetProcAddress(engineDLL, EAPI_NAME_RENDER_TEXT_2D);
    void* canvas = GetCanvas();

    if ((!callback) || (!canvas))
        return;

    callback(canvas, x, y, color, text, font, size, xAlign, yAlign, style, layout);
}

}