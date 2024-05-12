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

void* LoadTexture(const std::string& textureName)
{
    typedef void* (__thiscall* GetCanvasProto)(void*, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetCanvasProto callback = (GetCanvasProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_TEXTURE);
    void* graphicsEngine = GetGraphicsEngine();

    if ((!callback) || (!graphicsEngine))
        return nullptr;

    return callback(graphicsEngine, textureName);
}

void UnloadTexture(void* texture)
{
    typedef void (__thiscall* GetCanvasProto)(void*, void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    GetCanvasProto callback = (GetCanvasProto)GetProcAddress(engineDLL, EAPI_NAME_UNLOAD_TEXTURE);
    void* graphicsEngine = GetGraphicsEngine();

    if ((!callback) || (!graphicsEngine))
        return;

    callback(graphicsEngine, texture);
}

void RenderRect(const Rect& r1, const Rect& r2, void* renderTexture, const Color& color)
{
    typedef void (__thiscall* RenderRectProto)(void*, const Rect&, const Rect&, void*, const Color&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    RenderRectProto callback = (RenderRectProto)GetProcAddress(engineDLL, EAPI_NAME_RENDER_RECT);
    void* canvas = GetCanvas();

    if ((!callback) || (!canvas))
        return;

    callback(canvas, r1, r2, renderTexture, color);
}

void RenderText2D(int x, int y, const Color& color, const wchar_t* text, void* font, int size, GraphicsXAlign xAlign, GraphicsYAlign yAlign, int style, int layout)
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

uint32_t GetTextureWidth(void* texture)
{
    typedef uint32_t (__thiscall* GetTextureWidthProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return 0;

    GetTextureWidthProto callback = (GetTextureWidthProto)GetProcAddress(engineDLL, EAPI_NAME_GET_TEXTURE_WIDTH);
    if (!callback)
        return 0;

    return callback(texture);
}

uint32_t GetTextureHeight(void* texture)
{
    typedef uint32_t (__thiscall* GetTextureHeightProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return 0;

    GetTextureHeightProto callback = (GetTextureHeightProto)GetProcAddress(engineDLL, EAPI_NAME_GET_TEXTURE_HEIGHT);
    if (!callback)
        return 0;

    return callback(texture);
}

void* GetRenderTexture(void* texture)
{
    typedef void* (__thiscall* GetRenderTextureProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetRenderTextureProto callback = (GetRenderTextureProto)GetProcAddress(engineDLL, EAPI_NAME_GET_RENDER_TEXTURE);
    if (!callback)
        return nullptr;

    return callback(texture);
}

}