#ifndef INC_GDCL_DLL_ENGINE_API_UI_STYLE_MANAGER_H
#define INC_GDCL_DLL_ENGINE_API_UI_STYLE_MANAGER_H

#include <string>

namespace EngineAPI::UI
{

#if _WIN64
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPEAVStyleManager@2@XZ";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QEAAPEBVGraphicsFont2@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#else
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPAVStyleManager@2@XZ";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QAEPBVGraphicsFont2@2@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#endif

void* GetStyleManager();
void* LoadFontDirect(const std::string& fontName);

}

#endif//INC_GDCL_DLL_ENGINE_API_UI_STYLE_MANAGER_H