#ifndef INC_GDCL_DLL_ENGINE_API_SAVE_MANAGER_H
#define INC_GDCL_DLL_ENGINE_API_SAVE_MANAGER_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_DIRECT_READ[] = "?DirectRead@SaveManager@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAPEAXAEAI_N3@Z";
constexpr char EAPI_NAME_DIRECT_WRITE[] = "?DirectWrite@SaveManager@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBXI_N22@Z";
constexpr char EAPI_NAME_ADD_SAVE_JOB[] = "?AddJob@SaveManager@GAME@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBXI_N2@Z";
#else
constexpr char EAPI_NAME_DIRECT_READ[] = "?DirectRead@SaveManager@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AAPAXAAI_N3@Z";
constexpr char EAPI_NAME_DIRECT_WRITE[] = "?DirectWrite@SaveManager@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PBXI_N22@Z";
constexpr char EAPI_NAME_ADD_SAVE_JOB[] = "?AddJob@SaveManager@GAME@@QAEXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PBXI_N2@Z";
#endif

}


#endif//INC_GDCL_DLL_ENGINE_API_SAVE_MANAGER_H