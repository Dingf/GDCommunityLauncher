#include <Windows.h>
#include "EngineAPI.h"

namespace EngineAPI
{

void* GetFileSystem()
{
    typedef void* (__thiscall* GetFileSystemProto)(void*);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return nullptr;

    GetFileSystemProto callback = (GetFileSystemProto)GetProcAddress(engineDLL, EAPI_NAME_GET_FILESYSTEM);
    void** engine = GetEngineHandle();

    if ((!callback) || (!engine))
        return nullptr;

    return callback(*engine);
}

void GetFileList(std::vector<std::string>& fileList)
{
    typedef void (__thiscall* GetFileListProto)(void*, std::vector<std::string>&);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    GetFileListProto callback = (GetFileListProto)GetProcAddress(engineDLL, EAPI_NAME_GET_FILE_LIST);
    void* fileSystem = GetFileSystem();

    if ((!callback) || (!fileSystem))
        return;

    callback(fileSystem, fileList);
}

void AddFileSystemSource(FileSystemPartition partition, const std::string& path, const char* unk1, bool isModded, bool isResourcePath, bool unk2)
{
    typedef void (__thiscall* AddFileSystemSourceProto)(void*, FileSystemPartition, const std::string&, const char*, bool, bool, bool);

    HMODULE engineDLL = GetModuleHandle(TEXT(ENGINE_DLL));
    if (!engineDLL)
        return;

    AddFileSystemSourceProto callback = (AddFileSystemSourceProto)GetProcAddress(engineDLL, EAPI_NAME_ADD_FILESYSTEM_SOURCE);
    void* fileSystem = GetFileSystem();

    if ((!callback) || (!fileSystem))
        return;

    callback(fileSystem, partition, path, unk1, isModded, isResourcePath, unk2);
}

}