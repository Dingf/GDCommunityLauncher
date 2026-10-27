#ifndef INC_GDCL_DLL_GAME_API_ACHIEVEMENT_H
#define INC_GDCL_DLL_GAME_API_ACHIEVEMENT_H

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_UNLOCK_ACHIEVEMENT[] = "?Unlock@Achievement@GAME@@QEAAXXZ";
#else
constexpr char GAPI_NAME_UNLOCK_ACHIEVEMENT[] = "?Unlock@Achievement@GAME@@QAEXXZ";
#endif

}

#endif//INC_GDCL_DLL_GAME_API_ACHIEVEMENT_H