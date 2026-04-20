#ifndef INC_GDCL_DLL_ENGINE_API_LOCALIZE_H
#define INC_GDCL_DLL_ENGINE_API_LOCALIZE_H

#include <Windows.h>
#include "EngineAPI/Engine.h"

namespace EngineAPI::UI
{

#if _WIN64
constexpr char EAPI_NAME_GET_LOCALIZATION_MANAGER[] = "?Instance@LocalizationManager@GAME@@SAAEAV12@XZ";
constexpr char EAPI_NAME_LOAD_LOCALIZATION_MANAGER[] = "?Load@LocalizationManager@GAME@@UEAAXPEBD@Z";
constexpr char EAPI_NAME_LOCALIZE[] = "?Localize@LocalizationManager@GAME@@QEAAPEBGPEBDZZ";
constexpr char EAPI_NAME_LOAD_TAGS[] = "?LoadTags@LocalizationManager@GAME@@AEAAX_N@Z";
constexpr char EAPI_NAME_ADD_TAG[] = "?AddTagToMap@LocalizationManager@GAME@@MEAAXPEBD0_N@Z";
#else
constexpr char EAPI_NAME_GET_LOCALIZATION_MANAGER[] = "?Instance@LocalizationManager@GAME@@SAAAV12@XZ";
constexpr char EAPI_NAME_LOAD_LOCALIZATION_MANAGER[] = "?Load@LocalizationManager@GAME@@UAEXPBD@Z";
constexpr char EAPI_NAME_LOCALIZE[] = "?Localize@LocalizationManager@GAME@@QAAPBGPBDZZ";
constexpr char EAPI_NAME_LOAD_TAGS[] = "?LoadTags@LocalizationManager@GAME@@AAEX_N@Z";
constexpr char EAPI_NAME_ADD_TAG[] = "?AddTagToMap@LocalizationManager@GAME@@MAEXPBD0_N@Z";
#endif

void* GetLocalizationManager();
void LoadLocalizationTags(bool keepExisting);
void AddLocalizationTag(const char* tagLine, const char* filename, bool keepExisting);

template <typename... Ts>
const wchar_t* Localize(const char* tag, Ts... args)
{
    typedef const wchar_t* (__cdecl* LocalizeProto)(void*, const char*, Ts...);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    LocalizeProto callback = (LocalizeProto)GetProcAddress(engineDLL, EAPI_NAME_LOCALIZE);
    void* manager = GetLocalizationManager();

    if ((!callback) || (!manager))
        return nullptr;

    return callback(manager, tag, args...);
}

}

#endif//INC_GDCL_DLL_ENGINE_API_LOCALIZE_H