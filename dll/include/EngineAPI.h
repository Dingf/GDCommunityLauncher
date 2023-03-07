#ifndef INC_GDCL_DLL_ENGINE_API_H
#define INC_GDCL_DLL_ENGINE_API_H

#include <string>
#include <Windows.h>
#include "EngineAPI/Platform.h"
#include "EngineAPI/Color.h"
#include "EngineAPI/Rect.h"
#include "EngineAPI/GraphicsAlign.h"
#include "EngineAPI/KeyButtonEvent.h"
#include "EngineAPI/MouseEvent.h"
#include "EngineAPI/UI/ChatWindow.h"

namespace EngineAPI
{
#if _WIN64
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PEAVEngine@1@EA";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QEAAPEAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_PLAYER_LEVEL[] = "?GetPlayerLevel@GameInfo@GAME@@QEBAIXZ";
constexpr char EAPI_NAME_GET_HARDCORE[] = "?GetHardcore@GameInfo@GAME@@QEBA_NXZ";
constexpr char EAPI_NAME_GET_LEVEL_NAME[] = "?GetLevelName@GameInfo@GAME@@QEAAXAEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@@Z";
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QEBAPEAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QEAAAEAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_AREA_NAME_TAG[] = "?GetAreaNameTag@Engine@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_IS_MULTIPLAYER[] = "?GetIsMultiPlayer@GameInfo@GAME@@QEBA_NXZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPEBDXZ";
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPEAVStyleManager@2@XZ";
constexpr char EAPI_NAME_GET_OBJECT_MANAGER[] = "?Get@?$Singleton@VObjectManager@GAME@@@GAME@@SAPEAVObjectManager@2@XZ";
constexpr char EAPI_NAME_GET_LOCALIZATION_MANAGER[] = "?Instance@LocalizationManager@GAME@@SAAEAV12@XZ";
constexpr char EAPI_NAME_GET_OBJECT_ID[] = "?GetObjectId@Object@GAME@@QEBAIXZ";
constexpr char EAPI_NAME_GET_OBJECT_LIST[] = "?GetObjectList@ObjectManager@GAME@@QEBAXAEAV?$vector@PEBVObject@GAME@@@mem@@@Z";
constexpr char EAPI_NAME_GET_ENTITY_REGION[] = "?GetRegion@Entity@GAME@@QEBAPEAVRegion@2@XZ";
constexpr char EAPI_NAME_GET_REGION_ID[] = "?GetId@Region@GAME@@QEAAAEAVRegionId@2@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME[] = "?GetName@Region@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_SET_REGION_OF_NOTE[] = "?SetRegionOfNote@Engine@GAME@@QEAAXPEAVRegion@2@@Z";
constexpr char EAPI_NAME_DESTROY_OBJECT_EX[] = "?DestroyObjectEx@ObjectManager@GAME@@QEAAXPEAVObject@2@PEBDH@Z";
constexpr char EAPI_NAME_LOAD_WORLD[] = "?Load@World@GAME@@QEAA_NPEBD_N1@Z";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QEAAPEBVGraphicsFont2@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QEAAXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXHHAEBVColor@2@PEBGPEBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QEAAXVRect@2@PEBGAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_HANDLE_KEY_EVENT[] = "?HandleKeyEvent@DisplayWidget@GAME@@UEAA_NAEBVButtonEvent@InputDevice@2@@Z";
constexpr char EAPI_NAME_HANDLE_MOUSE_EVENT[] = "?HandleMouseEvent@DisplayWidget@GAME@@UEAA_NAEBUMouseEvent@InputDevice@2@@Z";
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QEAA_N_N0@Z";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QEAA_NPEBD@Z";
constexpr char EAPI_NAME_LOCALIZE[] = "?Localize@LocalizationManager@GAME@@QEAAPEBGPEBDZZ";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QAEPAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_PLAYER_LEVEL[] = "?GetPlayerLevel@GameInfo@GAME@@QBEIXZ";
constexpr char EAPI_NAME_GET_HARDCORE[] = "?GetHardcore@GameInfo@GAME@@QBE_NXZ";
constexpr char EAPI_NAME_GET_LEVEL_NAME[] = "?GetLevelName@GameInfo@GAME@@QAEXAAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@@Z";
constexpr char EAPI_NAME_GET_GRAPHICS_ENGINE[] = "?GetGraphicsEngine@Engine@GAME@@QBEPAVGraphicsEngine@2@XZ";
constexpr char EAPI_NAME_GET_CANVAS[] = "?GetCanvas@GraphicsEngine@GAME@@QAEAAVGraphicsCanvas@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_AREA_NAME_TAG[] = "?GetAreaNameTag@Engine@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_IS_MULTIPLAYER[] = "?GetIsMultiPlayer@GameInfo@GAME@@QBE_NXZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
constexpr char EAPI_NAME_GET_STYLE_MANAGER[] = "?Get@?$Singleton@VStyleManager@GAME@@@GAME@@SAPAVStyleManager@2@XZ";
constexpr char EAPI_NAME_GET_OBJECT_MANAGER[] = "?Get@?$Singleton@VObjectManager@GAME@@@GAME@@SAPAVObjectManager@2@XZ";
constexpr char EAPI_NAME_GET_LOCALIZATION_MANAGER[] = "?Instance@LocalizationManager@GAME@@SAAAV12@XZ";
constexpr char EAPI_NAME_GET_OBJECT_ID[] = "?GetObjectId@Object@GAME@@QBEIXZ";
constexpr char EAPI_NAME_GET_OBJECT_LIST[] = "?GetObjectList@ObjectManager@GAME@@QBEXAAV?$vector@PBVObject@GAME@@@mem@@@Z";
constexpr char EAPI_NAME_GET_ENTITY_REGION[] = "?GetRegion@Entity@GAME@@QBEPAVRegion@2@XZ";
constexpr char EAPI_NAME_GET_REGION_ID[] = "?GetId@Region@GAME@@QAEAAVRegionId@2@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME[] = "?GetName@Region@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_SET_REGION_OF_NOTE[] = "?SetRegionOfNote@Engine@GAME@@QAEXPAVRegion@2@@Z";
constexpr char EAPI_NAME_DESTROY_OBJECT_EX[] = "?DestroyObjectEx@ObjectManager@GAME@@QAEXPAVObject@2@PBDH@Z";
constexpr char EAPI_NAME_LOAD_WORLD[] = "?Load@World@GAME@@QAE_NPBD_N1@Z";
constexpr char EAPI_NAME_LOAD_FONT_DIRECT[] = "?LoadFontDirect@StyleManager@GAME@@QAEPBVGraphicsFont2@2@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_RENDER[] = "?Render@Engine@GAME@@QAEXXZ";
constexpr char EAPI_NAME_RENDER_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXHHABVColor@2@PBGPBVGraphicsFont2@2@HW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontStyleFlag@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_RENDER_STYLED_TEXT_2D[] = "?RenderText2d@GraphicsCanvas@GAME@@QAEXVRect@2@PBGABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GraphicsXAlign@2@W4GraphicsYAlign@2@W4FontLayout@2@@Z";
constexpr char EAPI_NAME_HANDLE_KEY_EVENT[] = "?HandleKeyEvent@DisplayWidget@GAME@@UAE_NABVButtonEvent@InputDevice@2@@Z";
constexpr char EAPI_NAME_HANDLE_MOUSE_EVENT[] = "?HandleMouseEvent@DisplayWidget@GAME@@UAE_NABUMouseEvent@InputDevice@2@@Z";
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QAE_N_N0@Z";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QAE_NPBD@Z";
constexpr char EAPI_NAME_LOCALIZE[] = "?Localize@LocalizationManager@GAME@@QAAPBGPBDZZ";
#endif

PULONG_PTR GetEngineHandle();
PULONG_PTR GetGameInfo();
uint32_t GetPlayerLevel();
bool GetHardcore();
std::wstring GetLevelName();
PULONG_PTR GetGraphicsEngine();
PULONG_PTR GetCanvas();
const char* GetModName();
const char* GetAreaNameTag();
bool IsMultiplayer();
PULONG_PTR GetStyleManager();
PULONG_PTR GetObjectManager();
PULONG_PTR GetLocalizationManager();
uint32_t GetObjectID(void* object);
PULONG_PTR GetEntityRegion(void* entity);
PULONG_PTR GetRegionID(void* region);
const char* GetRegionName(void* region);
void DestroyObjectEx(void* object);
PULONG_PTR LoadFontDirect(const std::string& fontName);
void RenderText2D(int x, int y, const Color& color, const wchar_t* text, PULONG_PTR font, int size, GraphicsXAlign xAlign, GraphicsYAlign yAlign, int style, int layout);

template <typename... T>
const wchar_t* Localize(const char* tag, T... args)
{
    typedef const wchar_t* (__cdecl* LocalizeProto)(void*, const char*, ...);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    LocalizeProto callback = (LocalizeProto)GetProcAddress(engineDLL, EAPI_NAME_LOCALIZE);
    PULONG_PTR manager = GetLocalizationManager();

    if ((!callback) || (!manager))
        return nullptr;

    return callback(manager, tag, args...);
}

}

#endif//INC_GDCL_DLL_ENGINE_API_H
