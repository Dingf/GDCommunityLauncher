#include "ClientHandler.h"
#include "EventManager.h"

bool HandleDirectRead(void* _this, const std::string& filename, void*& data, uint32_t& size, bool unk1, bool unk2)
{
    typedef bool (__thiscall* DirectReadProto)(void*, const std::string&, void*&, uint32_t&, bool, bool);

    DirectReadProto callback = (DirectReadProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_DIRECT_READ);
    if (callback)
    {
        size = 0;
        EventManager::Publish(GDCL_EVENT_DIRECT_FILE_READ, filename, &data, &size);
        return (size > 0) ? true : callback(_this, filename, data, size, unk1, unk2);
    }
    return false;
}

bool HandleDirectWrite(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2, bool unk3)
{
    typedef bool (__thiscall* DirectReadProto)(void*, const std::string&, void*, uint32_t, bool, bool, bool);

    DirectReadProto callback = (DirectReadProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_DIRECT_WRITE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_DIRECT_FILE_WRITE, filename, data, size);
        return callback(_this, filename, data, size, unk1, unk2, unk3);
    }
    return false;
}

void HandleAddSaveJob(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2)
{
    typedef void (__thiscall* AddSaveJobProto)(void*, const std::string&, void*, uint32_t, bool, bool);

    AddSaveJobProto callback = (AddSaveJobProto)HookManager::GetOriginalFunction(ENGINE_DLL, EngineAPI::EAPI_NAME_ADD_SAVE_JOB);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_ADD_SAVE_JOB, filename, data, size);
        callback(_this, filename, data, size, unk1, unk2);
    }
}