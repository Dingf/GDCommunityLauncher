#include "EngineAPI.h"

namespace EngineAPI
{

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

uint32_t GetPlayerLevel()
{
    typedef uint32_t(__thiscall* GetPlayerLevelProto)(PULONG_PTR);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return false;

    GetPlayerLevelProto callback = (GetPlayerLevelProto)GetProcAddress(engineDLL, EAPI_NAME_GET_PLAYER_LEVEL);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return 0;

    return callback(gameInfo);
}

bool GetHardcore()
{
    typedef bool(__thiscall* GetHardcoreProto)(PULONG_PTR);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return false;

    GetHardcoreProto callback = (GetHardcoreProto)GetProcAddress(engineDLL, EAPI_NAME_GET_HARDCORE);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    return callback(gameInfo);
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
    //TODO: Verify if this behavior exists/is compatible with 32 bit versions
    if (((ULONGLONG)result & 0xFFFFFFFF00000000L) == ((ULONGLONG)(*result) & 0xFFFFFFFF00000000L))
        return (const char*)(*result);
    else
        return (const char*)result;
}

bool IsMultiplayer()
{
    typedef bool(__thiscall* GetIsMultiplayerProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetIsMultiplayerProto callback = (GetIsMultiplayerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_IS_MULTIPLAYER);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return nullptr;

    return callback(gameInfo);
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