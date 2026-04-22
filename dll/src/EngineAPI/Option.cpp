#include "EngineAPI.h"

namespace EngineAPI
{

void* GetOptions()
{
    typedef void* (__thiscall* GetOptionsProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetOptionsProto callback = (GetOptionsProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OPTIONS);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback(*engine);
}

bool GetBoolOption(uint32_t index)
{
    typedef bool (__thiscall* GetBoolOptionProto)(void*, uint32_t);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return false;

    GetBoolOptionProto callback = (GetBoolOptionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_BOOL_OPTION);
    void* options = GetOptions();

    if ((!callback) || (!options))
        return false;

    return callback(options, index);
}

int32_t GetIntOption(uint32_t index)
{
    typedef int32_t (__thiscall* GetIntOptionProto)(void*, uint32_t);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return 0;

    GetIntOptionProto callback = (GetIntOptionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_INT_OPTION);
    void* options = GetOptions();

    if ((!callback) || (!options))
        return 0;

    return callback(options, index);
}

float GetFloatOption(uint32_t index)
{
    typedef float (__thiscall* GetFloatOptionProto)(void*, uint32_t);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return 0.0f;

    GetFloatOptionProto callback = (GetFloatOptionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_FLOAT_OPTION);
    void* options = GetOptions();

    if ((!callback) || (!options))
        return 0.0f;

    return callback(options, index);
}

std::string GetStringOption(uint32_t index)
{
    typedef std::string (__thiscall* GetStringOptionProto)(void*, uint32_t);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return {};

    GetStringOptionProto callback = (GetStringOptionProto)GetProcAddress(engineDLL, EAPI_NAME_GET_STRING_OPTION);
    void* options = GetOptions();

    if ((!callback) || (!options))
        return {};

    return callback(options, index);
}

}