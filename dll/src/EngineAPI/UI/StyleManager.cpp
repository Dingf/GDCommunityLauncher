#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI::UI
{
    
void* GetStyleManager()
{
    typedef void* (__thiscall* GetStyleManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
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

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    LoadFontDirectProto callback = (LoadFontDirectProto)GetProcAddress(engineDLL, EAPI_NAME_LOAD_FONT_DIRECT);
    void* manager = GetStyleManager();

    if ((!callback) || (!manager))
        return nullptr;

    return callback(manager, fontName);
}

}