#ifndef INC_GDCL_DLL_ENGINE_API_ENGINE_H
#define INC_GDCL_DLL_ENGINE_API_ENGINE_H

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PEAVEngine@1@EA";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPEBDXZ";
#else
constexpr char EAPI_NAME_ENGINE[] = "?gEngine@GAME@@3PAVEngine@1@A";
constexpr char EAPI_NAME_GET_VERSION[] = "?GetVersion@Engine@GAME@@SAPBDXZ";
#endif

void** GetEngineHandle();

}

#endif//INC_GDCL_DLL_ENGINE_API_ENGINE_H