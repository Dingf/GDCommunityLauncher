#ifndef INC_GDCL_DLL_ENGINE_API_RENDER_H
#define INC_GDCL_DLL_ENGINE_API_RENDER_H

#include "EngineAPI/Color.h"
#include "EngineAPI/UI/GraphicsAlign.h"

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QEBAPEAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QEAAAEAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QEAAXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXHHAEBVColor@2@PEBGPEBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXVRect@2@PEBGAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
#else
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QBEPAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QAEAAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QAEXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXHHABVColor@2@PBGPBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXVRect@2@PBGABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
#endif

void* GetGraphicsEngine();
void* GetCanvas();
void RenderText2D(int x, int y, const Color& color, const wchar_t* text, void* font, int size, UI::GraphicsXAlign xAlign, UI::GraphicsYAlign yAlign, int style, int layout);

}

#endif//INC_GDCL_DLL_ENGINE_API_RENDER_H