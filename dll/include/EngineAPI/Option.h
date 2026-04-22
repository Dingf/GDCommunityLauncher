#ifndef INC_GDCL_DLL_ENGINE_API_OPTION_H
#define INC_GDCL_DLL_ENGINE_API_OPTION_H

#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_OPTIONS[] = "?GetOptions@Engine@GAME@@QEAAPEAVOptions@2@XZ";
constexpr char EAPI_NAME_GET_BOOL_OPTION[] = "?GetBool@Options@GAME@@QEBA_NW4BoolName@12@@Z";
constexpr char EAPI_NAME_GET_INT_OPTION[] = "?GetInt@Options@GAME@@QEBAHW4IntName@12@@Z";
constexpr char EAPI_NAME_GET_FLOAT_OPTION[] = "?GetFloat@Options@GAME@@QEBAMW4FloatName@12@@Z";
constexpr char EAPI_NAME_GET_STRING_OPTION[] = "?GetString@Options@GAME@@QEBA?BV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4StringName@12@@Z";
#else
constexpr char EAPI_NAME_GET_OPTIONS[] = "?GetOptions@Engine@GAME@@QAEPAVOptions@2@XZ";
constexpr char EAPI_NAME_GET_BOOL_OPTION[] = "?GetBool@Options@GAME@@QBE_NW4BoolName@12@@Z";
constexpr char EAPI_NAME_GET_INT_OPTION[] = "?GetInt@Options@GAME@@QBEHW4IntName@12@@Z";
constexpr char EAPI_NAME_GET_FLOAT_OPTION[] = "?GetFloat@Options@GAME@@QBEMW4FloatName@12@@Z";
constexpr char EAPI_NAME_GET_STRING_OPTION[] = "?GetString@Options@GAME@@QBE?BV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4StringName@12@@Z";
#endif

void* GetOptions();
bool GetBoolOption(uint32_t index);
int32_t GetIntOption(uint32_t index);
float GetFloatOption(uint32_t index);
std::string GetStringOption(uint32_t index);

}

#endif//INC_GDCL_DLL_ENGINE_API_OPTION_H