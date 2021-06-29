#include "EngineAPI.h"

namespace EngineAPI
{

const Color Color::BLUE      (0.224f, 0.667f, 0.808f, 1.0f);
const Color Color::GREEN     (0.063f, 0.918f, 0.365f, 1.0f);
const Color Color::RED       (1.000f, 0.258f, 0.000f, 1.0f);
const Color Color::WHITE     (1.000f, 1.000f, 1.000f, 1.0f);
const Color Color::YELLOW    (1.000f, 0.960f, 0.170f, 1.0f);
const Color Color::PURPLE    (0.738f, 0.579f, 0.776f, 1.0f);
const Color Color::ORANGE    (0.950f, 0.640f, 0.300f, 1.0f);
const Color Color::SILVER    (0.600f, 0.600f, 0.600f, 1.0f);
const Color Color::FUSHIA    (1.000f, 0.411f, 0.705f, 1.0f);
const Color Color::CYAN      (0.000f, 1.000f, 1.000f, 1.0f);
const Color Color::INDIGO    (0.350f, 0.010f, 0.600f, 1.0f);
const Color Color::AQUA      (0.500f, 1.000f, 0.831f, 1.0f);
const Color Color::MAROON    (0.500f, 0.000f, 0.000f, 1.0f);
const Color Color::KHAKI     (0.941f, 0.901f, 0.549f, 1.0f);
const Color Color::DARK_GRAY (0.100f, 0.100f, 0.100f, 1.0f);
const Color Color::TEAL      (0.000f, 1.000f, 0.820f, 1.0f);
const Color Color::OLIVE     (0.570f, 0.797f, 0.000f, 1.0f);
const Color Color::TAN       (0.898f, 0.847f, 0.698f, 1.0f);

PULONG_PTR GetEngineHandle()
{
    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    return (PULONG_PTR)GetProcAddress(engineDLL, EAPI_NAME_ENGINE);
}

PULONG_PTR GetGameInfo()
{
    typedef PULONG_PTR(__thiscall* GetGameInfoProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetGameInfoProto callback = (GetGameInfoProto)GetProcAddress(engineDLL, EAPI_NAME_GET_GAME_INFO);
    PULONG_PTR engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback((LPVOID)*engine);
}

PULONG_PTR GetGraphicsEngine()
{
    typedef PULONG_PTR(__thiscall* GetGraphicsEngineProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetGraphicsEngineProto callback = (GetGraphicsEngineProto)GetProcAddress(engineDLL, EAPI_NAME_GET_GRAPHICS_ENGINE);
    PULONG_PTR engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback((LPVOID)*engine);
}

PULONG_PTR GetCanvas()
{
    typedef PULONG_PTR(__thiscall* GetCanvasProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetCanvasProto callback = (GetCanvasProto)GetProcAddress(engineDLL, EAPI_NAME_GET_CANVAS);
    PULONG_PTR graphicsEngine = GetGraphicsEngine();

    if ((!callback) || (!graphicsEngine))
        return nullptr;

    return callback(graphicsEngine);
}

const char* GetModName()
{
    typedef PULONG_PTR(__thiscall* GetModNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetModNameProto callback = (GetModNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_MOD_NAME);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    PULONG_PTR result = callback(gameInfo);

    // For some reason Steam sometimes stores a pointer and sometimes just stores the string itself
    // It seems like it depends on the length of the mod name? 

    // If it's a pointer, then it should be relatively close in memory so compare the first 32 bits
    if (((ULONGLONG)result & 0xFFFFFFFF00000000L) == ((ULONGLONG)(*result) & 0xFFFFFFFF00000000L))
        return (const char*)(*result);
    else
        return (const char*)result;
}

PULONG_PTR GetStyleManager()
{
    typedef PULONG_PTR(__thiscall* GetStyleManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetStyleManagerProto callback = (GetStyleManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_STYLE_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

PULONG_PTR LoadFontDirect(const std::string& fontName)
{
    typedef PULONG_PTR(__thiscall* LoadFontDirectProto)(void*, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    LoadFontDirectProto callback = (LoadFontDirectProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_FONT_DIRECT);
    PULONG_PTR styleManager = GetStyleManager();

    if ((!callback) || (!styleManager))
        return nullptr;

    return callback(styleManager, fontName);
}

void RenderText2D(int x, int y, const Color& color, const wchar_t* text, PULONG_PTR font, int size, GraphicsXAlign xAlign, GraphicsYAlign yAlign, int style, int layout)
{
    typedef void(__thiscall* RenderText2DProto)(void*, int, int, const Color&, const void*, void*, int, int, int, int, int);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return;

    RenderText2DProto callback = (RenderText2DProto)GetProcAddress(engineDLL, EAPI_NAME_RENDER_TEXT_2D);
    PULONG_PTR canvas = GetCanvas();

    if ((!callback) || (!canvas))
        return;

    callback(canvas, x, y, color, text, font, size, xAlign, yAlign, style, layout);
}

}