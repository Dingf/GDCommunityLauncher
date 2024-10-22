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
constexpr char EAPI_NAME_IS_EXPANSION1_LOADED[] = "?IsExpansion1Loaded@Engine@GAME@@QEBA_NXZ";
constexpr char EAPI_NAME_IS_EXPANSION2_LOADED[] = "?IsExpansion2Loaded@Engine@GAME@@QEBA_NXZ";
constexpr char EAPI_NAME_IS_EXPANSION3_LOADED[] = "?IsExpansion3Loaded@Engine@GAME@@QEBA_NXZ";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
constexpr char EAPI_NAME_INITIALIZE_MOD[] = "?InitializeMod@Engine@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z";
constexpr char EAPI_NAME_LOAD_DATABASE[] = "?LoadDatabase@Engine@GAME@@AAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
constexpr char EAPI_NAME_IS_EXPANSION1_LOADED[] = "?IsExpansion1Loaded@Engine@GAME@@QBE_NXZ";
constexpr char EAPI_NAME_IS_EXPANSION2_LOADED[] = "?IsExpansion2Loaded@Engine@GAME@@QBE_NXZ";
constexpr char EAPI_NAME_IS_EXPANSION3_LOADED[] = "?IsExpansion3Loaded@Engine@GAME@@QBE_NXZ";
#endif

void** GetEngineHandle();
std::string GetVersionString();
bool InitializeMod(const std::string& modName);
bool LoadDatabase(const std::string& databaseName);
bool IsExpansion1Loaded();
bool IsExpansion2Loaded();
bool IsExpansion3Loaded();

}

#endif//INC_GDCL_DLL_ENGINE_API_ENGINE_H