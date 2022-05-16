#include "LuaAPI.h"
#include "HookManager.h"

#include "Log.h"

namespace LuaAPI
{

void* _state = NULL;

void lua_settop(void* L, int index)
{
    typedef void(__cdecl* LuaSetTopProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return;

    LuaSetTopProto callback = (LuaSetTopProto)GetProcAddress(luaDLL, "lua_settop");
    if (!callback)
        return;

    callback(L, index);
}

void lua_pop(void* L, int n)
{
    lua_settop(L, -(n)-1);
}

void lua_pushstring(void* L, const char* s)
{
    typedef void(__cdecl* LuaPushStringProto)(void*, const char*);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return;

    LuaPushStringProto callback = (LuaPushStringProto)GetProcAddress(luaDLL, "lua_pushstring");
    if (!callback)
        return;

    callback(L, s);
}

void lua_pushnil(void* L)
{
    typedef void(__cdecl* LuaPushNilProto)(void*);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return;

    LuaPushNilProto callback = (LuaPushNilProto)GetProcAddress(luaDLL, "lua_pushnil");
    if (!callback)
        return;

    callback(L);
}

int lua_gettop(void* L)
{
    typedef int(__cdecl* LuaGetTopProto)(void*);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return 0;

    LuaGetTopProto callback = (LuaGetTopProto)GetProcAddress(luaDLL, "lua_gettop");
    if (!callback)
        return 0;

    return callback(L);
}

void lua_getfield(void* L, int index, const char* k)
{
    typedef void(__cdecl* LuaGetFieldProto)(void*, int, const char*);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return;

    LuaGetFieldProto callback = (LuaGetFieldProto)GetProcAddress(luaDLL, "lua_getfield");
    if (!callback)
        return;

    callback(L, index, k);
}

void lua_getglobal(void* L, const char* k)
{
    lua_getfield(L, -10002, k);
}

void lua_gettable(void* L, int index)
{
    typedef int(__cdecl* LuaGetTableProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return;

    LuaGetTableProto callback = (LuaGetTableProto)GetProcAddress(luaDLL, "lua_gettable");
    if (!callback)
        return;

    callback(L, index);
}

int lua_next(void* L, int index)
{
    typedef int(__cdecl* LuaGetNextProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return 0;

    LuaGetNextProto callback = (LuaGetNextProto)GetProcAddress(luaDLL, "lua_next");
    if (!callback)
        return 0;

    return callback(L, index);
}

int lua_type(void* L, int index)
{
    typedef int(__cdecl* LuaGetTypeProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return LUA_TNONE;

    LuaGetTypeProto callback = (LuaGetTypeProto)GetProcAddress(luaDLL, "lua_type");
    if (!callback)
        return LUA_TNONE;

    return callback(L, index);
}

bool lua_toboolean(void* L, int index)
{
    typedef int(__cdecl* LuaToBooleanProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return 0;

    LuaToBooleanProto callback = (LuaToBooleanProto)GetProcAddress(luaDLL, "lua_toboolean");
    if (!callback)
        return 0;

    return (bool)callback(L, index);
}

const char* lua_tolstring(void* L, int index, size_t* len)
{
    typedef const char*(__cdecl* LuaToStringProto)(void*, int, size_t*);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return nullptr;

    LuaToStringProto callback = (LuaToStringProto)GetProcAddress(luaDLL, "lua_tolstring");
    if (!callback)
        return nullptr;

    return callback(L, index, len);
}

const char* lua_tostring(void* L, int index)
{
    return lua_tolstring(L, index, NULL);
}

ptrdiff_t lua_tointeger(void* L, int index)
{
    typedef ptrdiff_t(__cdecl* LuaToIntegerProto)(void*, int);

    HMODULE luaDLL = GetModuleHandle(TEXT("lua51.dll"));
    if (!luaDLL)
        return 0;

    LuaToIntegerProto callback = (LuaToIntegerProto)GetProcAddress(luaDLL, "lua_tointeger");
    if (!callback)
        return 0;

    return callback(L, index);
}

int HandleLuaGetTop(void* L)
{
    typedef int(__cdecl *HandleLuaGetTopProto)(void*);

    HandleLuaGetTopProto callback = (HandleLuaGetTopProto)HookManager::GetOriginalFunction("lua51.dll", "lua_gettop");
    if (callback)
    {
       if (_state == NULL)
            _state = L;

        return callback(L);
    }
    return 0;
}

void Initialize()
{
    HookManager::CreateHook("lua51.dll", "lua_gettop", &HandleLuaGetTop);
}

void Cleanup()
{
    HookManager::DeleteHook("lua51.dll", "lua_gettop");
    _state = NULL;
}

void* GetState()
{
    return _state;
}

}