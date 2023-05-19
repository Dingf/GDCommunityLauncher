#ifndef INC_GDCL_DLL_ENGINE_API_LUA_H
#define INC_GDCL_DLL_ENGINE_API_LUA_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QEAA_N_N0@Z";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QEAA_NPEBD@Z";
#else
constexpr char EAPI_NAME_LUA_INITIALIZE[] = "?Initialize@LuaManager@GAME@@QAE_N_N0@Z";
constexpr char EAPI_NAME_LUA_RUN_CODE[] = "?RunCode@LuaManager@GAME@@QAE_NPBD@Z";
#endif

}

#endif//INC_GDCL_DLL_ENGINE_API_LUA_H