#ifndef INC_GDCL_DLL_ENGINE_API_OBJECT_H
#define INC_GDCL_DLL_ENGINE_API_OBJECT_H

#include <vector>
#include <string>

namespace EngineAPI
{

#if _WIN64
constexpr char EAPI_NAME_GET_OBJECT_MANAGER[] = "?Get@?$Singleton@VObjectManager@GAME@@@GAME@@SAPEAVObjectManager@2@XZ";
constexpr char EAPI_NAME_GET_OBJECT_ID[] = "?GetObjectId@Object@GAME@@QEBAIXZ";
constexpr char EAPI_NAME_GET_OBJECT_NAME[] = "?GetObjectName@Object@GAME@@QEBAPEBDXZ";
constexpr char EAPI_NAME_GET_OBJECT_LIST[] = "?GetObjectList@ObjectManager@GAME@@QEBAXAEAV?$vector@PEBVObject@GAME@@@mem@@@Z";
constexpr char EAPI_NAME_DESTROY_OBJECT_EX[] = "?DestroyObjectEx@ObjectManager@GAME@@QEAAXPEAVObject@2@PEBDH@Z";
#else
constexpr char EAPI_NAME_GET_OBJECT_MANAGER[] = "?Get@?$Singleton@VObjectManager@GAME@@@GAME@@SAPAVObjectManager@2@XZ";
constexpr char EAPI_NAME_GET_OBJECT_ID[] = "?GetObjectId@Object@GAME@@QBEIXZ";
constexpr char EAPI_NAME_GET_OBJECT_NAME[] = "?GetObjectName@Object@GAME@@QBEPBDXZ";
constexpr char EAPI_NAME_GET_OBJECT_LIST[] = "?GetObjectList@ObjectManager@GAME@@QBEXAAV?$vector@PBVObject@GAME@@@mem@@@Z";
constexpr char EAPI_NAME_DESTROY_OBJECT_EX[] = "?DestroyObjectEx@ObjectManager@GAME@@QAEXPAVObject@2@PBDH@Z";
#endif

void* GetObjectManager();
void GetObjectList(std::vector<void*>& objectList);
void* FindObjectByID(uint32_t objectID);
uint32_t GetObjectID(void* object);
std::string GetObjectName(void* object);
void DestroyObjectEx(void* object);

}

#endif//INC_GDCL_DLL_ENGINE_API_OBJECT_H