#ifndef INC_GDCL_DLL_ENGINE_API_H
#define INC_GDCL_DLL_ENGINE_API_H

#include <string>
#include <Windows.h>

namespace EngineAPI
{
#if _WIN64
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PEAVEngine@1@EA";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QEAAPEAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPEBDXZ";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_GAME_INFO[] = "?GetGameInfo@Engine@GAME@@QAEPAVGameInfo@2@XZ";
constexpr char EAPI_NAME_GET_MOD_NAME[] = "?GetModName@GameInfo@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
#endif

PULONG_PTR GetEngineHandle();

PULONG_PTR GetGameInfo();

const char* GetModName();

}

#endif//INC_GDCL_DLL_ENGINE_API_H
