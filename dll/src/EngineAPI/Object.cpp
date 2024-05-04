#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{
    
void* GetObjectManager()
{
    typedef void* (__thiscall* GetObjectManagerProto)();

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetObjectManagerProto callback = (GetObjectManagerProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_MANAGER);
    if (!callback)
        return nullptr;

    return callback();
}

void GetObjectList(std::vector<void*>& objectList)
{
    typedef void(__thiscall* GetObjectListProto)(void*, std::vector<void*>&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    GetObjectListProto callback = (GetObjectListProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_LIST);
    void* objectManager = GetObjectManager();
    if ((!callback) || (!objectManager))
        return;

    return callback(objectManager, objectList);
}

void* FindObjectByID(uint32_t objectID)
{
    std::vector<void*> objectList;
    GetObjectList(objectList);

    for (size_t i = 0; i < objectList.size(); ++i)
    {
        if (GetObjectID(objectList[i]) == objectID)
            return objectList[i];
    }
    return nullptr;
}

uint32_t GetObjectID(void* object)
{
    typedef uint32_t(__thiscall* GetObjectIDProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if ((!engineDLL) || (!object))
        return 0;

    GetObjectIDProto callback = (GetObjectIDProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_ID);
    if (!callback)
        return 0;

    return callback(object);
}

std::string GetObjectName(void* object)
{
    typedef const char* (__thiscall* GetObjectNameProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if ((!engineDLL) || (!object))
        return {};

    GetObjectNameProto callback = (GetObjectNameProto)GetProcAddress(engineDLL, EAPI_NAME_GET_OBJECT_NAME);
    if (!callback)
        return {};

    return callback(object);
}

void DestroyObjectEx(void* object)
{
    typedef void(__thiscall* DestroyObjectExProto)(void*, void*, const char*, int32_t);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    void* objectManager = GetObjectManager();
    if ((!engineDLL) || (!objectManager))
        return;

    DestroyObjectExProto callback = (DestroyObjectExProto)GetProcAddress(engineDLL, EAPI_NAME_DESTROY_OBJECT_EX);
    if (!callback)
        return;

    callback(objectManager, object, nullptr, 0);
}

uint32_t CreateObjectID()
{
    typedef uint32_t (__thiscall* CreateObjectIDProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    void* objectManager = GetObjectManager();
    if ((!engineDLL) || (!objectManager))
        return 0;

    CreateObjectIDProto callback = (CreateObjectIDProto)GetProcAddress(engineDLL, EAPI_NAME_CREATE_OBJECT_ID);
    if (!callback)
        return 0;

    return callback(objectManager);
}

}