#ifndef INC_GDCL_DLL_ENGINE_API_WORLD_H
#define INC_GDCL_DLL_ENGINE_API_WORLD_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_LOAD_WORLD[] = "?Load@World@GAME@@QEAA_NPEBD_N1@Z";
#else
constexpr char EAPI_NAME_LOAD_WORLD[] = "?Load@World@GAME@@QAE_NPBD_N1@Z";
#endif

}

#endif//INC_GDCL_DLL_ENGINE_API_WORLD_H