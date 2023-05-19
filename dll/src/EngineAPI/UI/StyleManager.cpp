#include <Windows.h>
#include "EngineAPI/UI/StyleManager.h"

namespace EngineAPI::UI
{
    
void* GetStyleManager()
{
    typedef void* (__thiscall* GetStyleManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    GetStyleManagerProto callback = (GetStyleManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_STYLE_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

void* LoadFontDirect(const std::string& fontName)
{
    typedef void* (__thiscall* LoadFontDirectProto)(void*, const std::string&);

    HMODULE engineDLL = GetModuleHandle(TEXT("Engine.dll"));
    if (!engineDLL)
        return nullptr;

    LoadFontDirectProto callback = (LoadFontDirectProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_FONT_DIRECT);
    void* styleManager = GetStyleManager();

    if ((!callback) || (!styleManager))
        return nullptr;

    return callback(styleManager, fontName);
}

}