#ifndef INC_GDCL_DLL_ENGINE_API_ENGINE_H
#define INC_GDCL_DLL_ENGINE_API_ENGINE_H

#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PEAVEngine@1@EA";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPEBDXZ";
constexpr char EAPI_NAME_INITIALIZE_MOD[] = "?InitializeMod@Engine@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z";
constexpr char EAPI_NAME_LOAD_DATABASE[] = "?LoadDatabase@Engine@GAME@@AEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
constexpr char EAPI_NAME_INITIALIZE_MOD[] = "?InitializeMod@Engine@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z";
constexpr char EAPI_NAME_LOAD_DATABASE[] = "?LoadDatabase@Engine@GAME@@AAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#endif

void** GetEngineHandle();
std::string GetVersionString();
bool InitializeMod(const std::string& modName);
bool LoadDatabase(const std::string& databaseName);
bool IsExpansion1Enabled();
bool IsExpansion2Enabled();
bool IsExpansion3Enabled();

}

#endif//INC_GDCL_DLL_ENGINE_API_ENGINE_H