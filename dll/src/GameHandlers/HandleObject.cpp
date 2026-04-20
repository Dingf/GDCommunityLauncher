#include "GameHandler.h"

uint32_t HandleGetObjectID(void* _this)
{
    typedef uint32_t (__thiscall* GetObjectIDProto)(void*);

    GetObjectIDProto callback = (GetObjectIDProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_GET_OBJECT_ID);
    if (callback)
    {
        uint32_t objectID = callback(_this);
        EngineAPI::CacheObjectID(_this, objectID);
        return objectID;
    }
    return 0;
}

void HandleDestroyObjectEx(void* _this, void* object, const char* unk1, int32_t unk2)
{
    typedef void(__thiscall* DestroyObjectExProto)(void*, void*, const char*, int32_t);

    DestroyObjectExProto callback = (DestroyObjectExProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_DESTROY_OBJECT_EX);
    if (callback)
    {
        if (object)
        {
            uint32_t objectID = *(uint32_t*)((uint8_t*)object + 0x2C);
            EngineAPI::ClearCachedObjectID(objectID);
        }
        callback(_this, object, unk1, unk2);
    }
}