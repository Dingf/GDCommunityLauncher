#ifndef INC_GDCL_DLL_ENGINE_API_ACTOR_H
#define INC_GDCL_DLL_ENGINE_API_ACTOR_H

#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_ACTOR_DESCRIPTION_TAG[] = "?GetDescriptionTag@Actor@GAME@@QEBAPEBDXZ";
#else
constexpr char EAPI_NAME_GET_ACTOR_DESCRIPTION_TAG[] = "?GetDescriptionTag@Actor@GAME@@QBEPBDXZ";
#endif

std::string GetActorDescriptionTag(void* actor);

}

#endif//INC_GDCL_DLL_ENGINE_API_ACTOR_H