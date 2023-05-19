#include "ClientHandlers.h"
#include "EventManager.h"

bool HandleDirectRead(void* _this, const std::string& filename, void*& data, uint32_t& size, bool unk1, bool unk2)
{
    typedef bool (__thiscall* DirectReadProto)(void*, const std::string&, void*&, uint32_t&, bool, bool);

    DirectReadProto callback = (DirectReadProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_READ);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_DIRECT_FILE_READ, (void*)&filename);
        return callback(_this, filename, data, size, unk1, unk2);
    }
    return false;
}

bool HandleDirectWrite(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2, bool unk3)
{
    typedef bool (__thiscall* DirectReadProto)(void*, const std::string&, void*, uint32_t, bool, bool, bool);

    DirectReadProto callback = (DirectReadProto)HookManager::GetOriginalFunction("Engine.dll", EngineAPI::EAPI_NAME_DIRECT_WRITE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_DIRECT_FILE_WRITE, (void*)&filename);
        return callback(_this, filename, data, size, unk1, unk2, unk3);
    }
    return false;
}