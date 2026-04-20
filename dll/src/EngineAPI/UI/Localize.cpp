#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI::UI
{

void* GetLocalizationManager()
{
    typedef void* (__thiscall* GetLocalizationManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetLocalizationManagerProto callback = (GetLocalizationManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_LOCALIZATION_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

void LoadLocalizationTags(bool keepExisting)
{
    typedef void* (__thiscall* LoadLocalizationTagsProto)(void*, bool);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    LoadLocalizationTagsProto callback = (LoadLocalizationTagsProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_TAGS);
    void* manager = GetLocalizationManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, keepExisting);
}


void AddLocalizationTag(const char* tagLine, const char* filename, bool keepExisting)
{
    typedef void* (__thiscall* AddLocalizationTagProto)(void*, const char*, const char*, bool);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    AddLocalizationTagProto callback = (AddLocalizationTagProto)GetProcAddress(engineDLL, EAPI_NAME_ADD_TAG);
    void* manager = GetLocalizationManager();

    if ((!callback) || (!manager))
        return;

    callback(manager, tagLine, filename, keepExisting);
}

}