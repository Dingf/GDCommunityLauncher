#include "ClientHandler.h"
#include "EventManager.h"

bool HandleDeleteFile(const char* filename)
{
    typedef BOOL (__cdecl* DeleteFileProto)(LPCSTR);

    DeleteFileProto callback = (DeleteFileProto)HookManager::GetOriginalFunction(KERNEL_DLL, WindowsAPI::WAPI_NAME_DELETE_FILE);
    if (callback)
    {
        EventManager::Publish(GDCL_EVENT_DELETE_FILE, filename);
        return callback(filename);
    }
    return false;
}