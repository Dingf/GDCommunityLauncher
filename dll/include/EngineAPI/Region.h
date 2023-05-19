#ifndef INC_GDCL_DLL_ENGINE_API_REGION_H
#define INC_GDCL_DLL_ENGINE_API_REGION_H

#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_ENTITY_REGION[] = "?GetRegion@Entity@GAME@@QEBAPEAVRegion@2@XZ";
constexpr char EAPI_NAME_GET_REGION_ID[] = "?GetId@Region@GAME@@QEAAAEAVRegionId@2@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME[] = "?GetName@Region@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME_TAG[] = "?GetAreaNameTag@Engine@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_SET_REGION_OF_NOTE[] = "?SetRegionOfNote@Engine@GAME@@QEAAXPEAVRegion@2@@Z";
#else
constexpr char EAPI_NAME_GET_ENTITY_REGION[] = "?GetRegion@Entity@GAME@@QBEPAVRegion@2@XZ";
constexpr char EAPI_NAME_GET_REGION_ID[] = "?GetId@Region@GAME@@QAEAAVRegionId@2@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME[] = "?GetName@Region@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_GET_REGION_NAME_TAG[] = "?GetAreaNameTag@Engine@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
constexpr char EAPI_NAME_SET_REGION_OF_NOTE[] = "?SetRegionOfNote@Engine@GAME@@QAEXPAVRegion@2@@Z";
#endif

void* GetEntityRegion(void* entity);
void* GetRegionID(void* region);
std::string GetRegionName(void* region);
std::string GetRegionNameTag();

}

#endif//INC_GDCL_DLL_ENGINE_API_REGION_H