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

std::wstring GetLevelName()
{
    typedef void(__thiscall* GetLevelNameProto)(PULONG_PTR, std::wstring&);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return {};

    GetLevelNameProto callback = (GetLevelNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_LEVEL_NAME);
    PULONG_PTR gameInfo = GetGameInfo();

    if ((!callback) || (!gameInfo))
        return {};

    std::wstring result;
    callback(gameInfo, result);
    return result;
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
    if (((ULONGLONG)result & 0xFFFFFFFF00000000L) == ((ULONGLONG)(*result) & 0xFFFFFFFF00000000L))
        return (const char*)(*result);
    else
        return (const char*)result;
}

const char* GetAreaNameTag()
{
    typedef PULONG_PTR(__thiscall* GetModNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetModNameProto callback = (GetModNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_AREA_NAME_TAG);
    PULONG_PTR engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    PULONG_PTR result = callback((LPVOID)*engine);
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

PULONG_PTR GetObjectManager()
{
    typedef PULONG_PTR(__thiscall* GetObjectManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetObjectManagerProto callback = (GetObjectManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

PULONG_PTR GetLocalizationManager()
{
    typedef PULONG_PTR(__thiscall* GetLocalizationManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetLocalizationManagerProto callback = (GetLocalizationManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_LOCALIZATION_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

PULONG_PTR GetEntityRegion(LPVOID entity)
{
    typedef PULONG_PTR(__thiscall* GetEntityRegionProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetEntityRegionProto callback = (GetEntityRegionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_ENTITY_REGION);
    if (!callback)
        return nullptr;

    return callback(entity);
}

PULONG_PTR GetRegionID(LPVOID region)
{
    typedef PULONG_PTR(__thiscall* GetRegionIDProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetRegionIDProto callback = (GetRegionIDProto)GetProcAddress(engineDLL, EAPI_NAME_GET_REGION_ID);
    if (!callback)
        return nullptr;

    return callback(region);
}

const char* GetRegionName(LPVOID region)
{
    typedef PULONG_PTR(__thiscall* GetRegionNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetRegionNameProto callback = (GetRegionNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_REGION_NAME);
    if (!callback)
        return nullptr;

    PULONG_PTR result = callback(region);
    if (((ULONGLONG)result & 0xFFFFFFFF00000000L) == ((ULONGLONG)(*result) & 0xFFFFFFFF00000000L))
        return (const char*)(*result);
    else
        return (const char*)result;
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