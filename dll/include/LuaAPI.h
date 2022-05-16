#ifndef INC_GDCL_DLL_LUA_API_H
#define INC_GDCL_DLL_LUA_API_H

#include <cstddef>

namespace LuaAPI
{

enum LuaType
{
    LUA_TNONE = -1,
    LUA_TNIL = 0,
    LUA_TBOOLEAN = 1,
    LUA_TLIGHTUSERDATA = 2,
    LUA_TNUMBER = 3,
    LUA_TSTRING = 4,
    LUA_TTABLE = 5,
    LUA_TFUNCTION = 6,
    LUA_TUSERDATA = 7,
    LUA_TTHREAD = 8,
};

void lua_settop(void* L, int index);
void lua_pop(void* L, int n);
void lua_pushstring(void* L, const char* s);
void lua_pushnil(void* L);
int lua_gettop(void* L);
void lua_getfield(void* L, int index, const char* k);
void lua_getglobal(void* L, const char* k);
void lua_gettable(void* L, int index);
int lua_next(void* L, int index);
int lua_type(void* L, int index);
bool lua_toboolean(void* L, int index);
const char* lua_tolstring(void* L, int index, size_t* len);
const char* lua_tostring(void* L, int index);
ptrdiff_t lua_tointeger(void* L, int index);

void Initialize();
void Cleanup();
void* GetState();

}

#endif//INC_GDCL_DLL_LUA_API_H
