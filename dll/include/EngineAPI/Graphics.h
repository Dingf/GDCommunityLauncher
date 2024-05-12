#ifndef INC_GDCL_DLL_ENGINE_API_UI_GRAPHICS_H
#define INC_GDCL_DLL_ENGINE_API_UI_GRAPHICS_H

#include <stdint.h>
#include "EngineAPI/Color.h"
#include "EngineAPI/Rect.h"

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QEBAPEAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QEAAAEAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_LOAD_TEXTURE[] = "?LoadTexture@GraphicsEngine@GAME@@QEAAPEBVGraphicsTexture@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_UNLOAD_TEXTURE[] = "?UnloadTexture@GraphicsEngine@GAME@@QEAAXPEBVGraphicsTexture@2@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QEAAXXZ";
constexpr char EAPI_NAME_RENDER_RECT[] = "?RenderRect@GraphicsCanvas@GAME@@QEAAXAEBVRect@2@0PEBVRenderTexture@2@AEBVColor@2@@Z";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXHHAEBVColor@2@PEBGPEBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXVRect@2@PEBGAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_GET_TEXTURE_WIDTH[] = "?GetWidth@GraphicsTexture@GAME@@QEBAHXZ";
constexpr char EAPI_NAME_GET_TEXTURE_HEIGHT[] = "?GetHeight@GraphicsTexture@GAME@@QEBAHXZ";
constexpr char EAPI_NAME_GET_RENDER_TEXTURE[] = "?GetTexture@GraphicsTexture@GAME@@QEBAPEBVRenderTexture@2@XZ";
#else
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QBEPAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QAEAAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_LOAD_TEXTURE[] = "?LoadTexture@GraphicsEngine@GAME@@QAEPBVGraphicsTexture@2@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_UNLOAD_TEXTURE[] = "?UnloadTexture@GraphicsEngine@GAME@@QAEXPBVGraphicsTexture@2@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QAEXXZ";
constexpr char EAPI_NAME_RENDER_RECT[] = "?RenderRect@GraphicsCanvas@GAME@@QAEXABVRect@2@0PBVRenderTexture@2@ABVColor@2@@Z";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXHHABVColor@2@PBGPBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXVRect@2@PBGABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_GET_TEXTURE_WIDTH[] = "?GetWidth@GraphicsTexture@GAME@@QBEHXZ";
constexpr char EAPI_NAME_GET_TEXTURE_HEIGHT[] = "?GetHeight@GraphicsTexture@GAME@@QBEHXZ";
constexpr char EAPI_NAME_GET_RENDER_TEXTURE[] = "?GetTexture@GraphicsTexture@GAME@@QBEPBVRenderTexture@2@XZ";
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

void* GetGraphicsEngine();
void* GetCanvas();
void* LoadTexture(const std::string& textureName);
void UnloadTexture(void* texture);
void RenderRect(const Rect& r1, const Rect& r2, void* renderTexture, const Color& color);
void RenderText2D(int x, int y, const Color& color, const wchar_t* text, void* font, int32_t size, GraphicsXAlign xAlign, GraphicsYAlign yAlign, int32_t style, int32_t layout);
uint32_t GetTextureWidth(void* texture);
uint32_t GetTextureHeight(void* texture);
void* GetRenderTexture(void* texture);

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_GRAPHICS_H