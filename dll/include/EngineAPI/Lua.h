#ifndef INC_GDCL_DLL_ENGINE_API_LUA_H
#define INC_GDCL_DLL_ENGINE_API_LUA_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QEAA_N_N0@Z";
constexpr char EAPI_NAME_LUA_SHUTDOWN[] = "?Shutdown@LuaManager@GAME@@QEAAXXZ";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QEAA_NPEBD@Z";
constexpr char EAPI_NAME_LUA_LOAD_FILE[] = "?Load@LuaManager@GAME@@QEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#else
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QAE_N_N0@Z";
constexpr char EAPI_NAME_LUA_SHUTDOWN[] = "?Shutdown@LuaManager@GAME@@QAEXXZ";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QAE_NPBD@Z";
constexpr char EAPI_NAME_LUA_LOAD_FILE[] = "?Load@LuaManager@GAME@@QAE_NABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
#endif

void SetLuaManager(void* manager);
void* GetLuaManager();
bool RunLuaCode(const char* code);
bool LoadLuaFile(const std::string& filename);

}

#endif//INC_GDCL_DLL_ENGINE_API_LUA_H