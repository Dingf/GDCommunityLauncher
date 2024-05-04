#ifndef INC_GDCL_DLL_ENGINE_API_FILE_SYSTEM_H
#define INC_GDCL_DLL_ENGINE_API_FILE_SYSTEM_H

#include <vector>
#include <string>

namespace EngineAPI
{

enum FileSystemPartition
{
    PARTITION_SETTINGS = 0,
    PARTITION_MOD_RESOURCES = 1,
    PARTITION_BASE_RESOURCES = 2,
};

#if _WIN64
constexpr char EAPI_NAME_GET_FILESYSTEM[] = "?GetFileSystem@Engine@GAME@@QEAAPEAVFileSystem@2@XZ";
constexpr char EAPI_NAME_GET_FILE_LIST[] = "?GetFileList@FileSystem@GAME@@QEAAXAEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@mem@@@Z";
constexpr char EAPI_NAME_ADD_FILESYSTEM_SOURCE[] = "?AddSource@FileSystem@GAME@@QEAA_NW4Partition@12@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBD_N33@Z";
#else
constexpr char EAPI_NAME_GET_FILESYSTEM[] = "?GetFileSystem@Engine@GAME@@QAEPAVFileSystem@2@XZ";
constexpr char EAPI_NAME_GET_FILE_LIST[] = "?GetFileList@FileSystem@GAME@@QAEXAAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@mem@@@Z";
constexpr char EAPI_NAME_ADD_FILESYSTEM_SOURCE[] = "?AddSource@FileSystem@GAME@@QAE_NW4Partition@12@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PBD_N33@Z";
#endif

void* GetFileSystem();
void GetFileList(std::vector<std::string>& fileList);
void AddFileSystemSource(FileSystemPartition partition, const std::string& path, const char* unk1, bool isModded, bool isResourcePath, bool unk2);

}


#endif//INC_GDCL_DLL_ENGINE_API_FILE_SYSTEM_H