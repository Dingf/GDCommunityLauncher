#ifndef INC_GDCL_DLL_ENGINE_API_H
#define INC_GDCL_DLL_ENGINE_API_H

#include <string>
#include <Windows.h>

namespace EngineAPI
{
#if _WIN64
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PEAVEngine@1@EA";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QEAAPEAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QEBAPEAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QEAAAEAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPEBDXZ";
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPEAVStyleManager@2@XZ";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QEAAPEBVGraphicsFont2@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QEAAXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXHHAEBVColor@2@PEBGPEBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QAEPAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QBEPAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QAEAAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPAVStyleManager@2@XZ";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QAEPBVGraphicsFont2@2@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QAEXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXHHABVColor@2@PBGPBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
#endif

enum GraphicsXAlign
{
    GRAPHICS_X_ALIGN_LEFT = 0,
    GRAPHICS_X_ALIGN_RIGHT = 1,
    GRAPHICS_X_ALIGN_CENTER = 2,
};

enum GraphicsYAlign
{
    GRAPHICS_Y_ALIGN_TOP = 0,
    GRAPHICS_Y_ALIGN_BOTTOM = 1,
    GRAPHICS_Y_ALIGN_CENTER = 2,
};

struct Color
{
    Color() : _r(1.0), _g(1.0), _b(1.0), _a(1.0) {}
    Color(float r, float g, float b, float a) : _r(r), _g(g), _b(b), _a(a) {}

    static const Color BLUE;
    static const Color GREEN;
    static const Color RED;
    static const Color WHITE;
    static const Color YELLOW;
    static const Color PURPLE;
    static const Color ORANGE;
    static const Color SILVER;
    static const Color FUSHIA;
    static const Color CYAN;
    static const Color INDIGO;
    static const Color AQUA;
    static const Color MAROON;
    static const Color KHAKI;
    static const Color DARK_GRAY;
    static const Color TEAL;
    static const Color OLIVE;
    static const Color TAN;

    float _r;
    float _g;
    float _b;
    float _a;
};

PULONG_PTR GetEngineHandle();

PULONG_PTR GetGameInfo();

PULONG_PTR GetGraphicsEngine();

PULONG_PTR GetCanvas();

const char* GetModName();

PULONG_PTR GetStyleManager();

PULONG_PTR LoadFontDirect(const std::string& fontName);

void RenderText2D(int x, int y, const Color& color, const wchar_t* text, PULONG_PTR font, int size, GraphicsXAlign xAlign, GraphicsYAlign yAlign, int style, int layout);

}

#endif//INC_GDCL_DLL_ENGINE_API_H
