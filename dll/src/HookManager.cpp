#include <string>
#include <Windows.h>
#include "HookManager.h"

HookManager& HookManager::GetInstance()
{
    static HookManager instance;
    return instance;
}

LPVOID HookManager::GetOriginalFunction(LPCSTR moduleName, LPCSTR functionName)
{
    HookManager& instance = GetInstance();
    auto it = instance._originalFunctions.find({ moduleName, functionName });
    if (it != instance._originalFunctions.end())
    {
        return it->second;
    }
    return NULL;
}

LPVOID HookManager::CreateHook(LPCSTR moduleName, LPCSTR functionName, PVOID function)
{
    HookManager& instance = GetInstance();
    ExportKey key(moduleName, functionName);
    if (instance._originalFunctions.count(key) > 0)
        return NULL;

    DWORD_PTR start = (DWORD_PTR)GetModuleHandleA(NULL);
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)start;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(start + dosHeader->e_lfanew);

    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
        return NULL;

    std::string moduleString(moduleName);
    std::string functionString(functionName);
    IMAGE_DATA_DIRECTORY imports = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    for (PIMAGE_IMPORT_DESCRIPTOR descriptor = (PIMAGE_IMPORT_DESCRIPTOR)(start + imports.VirtualAddress); descriptor->Name != NULL; descriptor++)
    {
        if ((LPCSTR)(start + descriptor->Name) != moduleString)
            continue;

        if ((descriptor->FirstThunk == NULL) || (descriptor->OriginalFirstThunk == NULL))
            return NULL;

        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(start + descriptor->FirstThunk);
        PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA)(start + descriptor->OriginalFirstThunk);

        for (; originalThunk->u1.AddressOfData != NULL; originalThunk++, thunk++)
        {
            PIMAGE_IMPORT_BY_NAME importData = (PIMAGE_IMPORT_BY_NAME)(start + originalThunk->u1.AddressOfData);
            if ((LPCSTR)importData->Name != functionString)
                continue;

            DWORD originalProtect;
            LPVOID originalFunction = (LPVOID)thunk->u1.Function;

            if (!VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &originalProtect))
                return NULL;

            thunk->u1.Function = (DWORD_PTR)function;

            if (!VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), originalProtect, &originalProtect))
                return NULL;

            instance._originalFunctions[key] = originalFunction;
            return originalFunction;
        }
    }

    return NULL;
}

BOOL HookManager::DeleteHook(LPCSTR moduleName, LPCSTR functionName)
{
    HookManager& instance = GetInstance();
    LPVOID originalFunction = instance.GetOriginalFunction(moduleName, functionName);
    if (originalFunction == NULL)
        return FALSE;

    DWORD_PTR start = (DWORD_PTR)GetModuleHandleA(NULL);
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)start;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(start + dosHeader->e_lfanew);

    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    std::string moduleString(moduleName);
    std::string functionString(functionName);
    IMAGE_DATA_DIRECTORY imports = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    for (PIMAGE_IMPORT_DESCRIPTOR descriptor = (PIMAGE_IMPORT_DESCRIPTOR)(start + imports.VirtualAddress); descriptor->Name != NULL; descriptor++)
    {
        if ((LPCSTR)(start + descriptor->Name) != moduleString)
            continue;

        if ((descriptor->FirstThunk == NULL) || (descriptor->OriginalFirstThunk == NULL))
            return FALSE;

        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(start + descriptor->FirstThunk);
        PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA)(start + descriptor->OriginalFirstThunk);

        for (; originalThunk->u1.AddressOfData != NULL; originalThunk++, thunk++)
        {
            PIMAGE_IMPORT_BY_NAME importData = (PIMAGE_IMPORT_BY_NAME)(start + originalThunk->u1.AddressOfData);
            if ((LPCSTR)importData->Name != functionString)
                continue;

            DWORD originalProtect;
            if (!VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &originalProtect))
                return FALSE;

            thunk->u1.Function = (DWORD_PTR)originalFunction;
            instance._originalFunctions.erase({ moduleName, functionName });

            if (VirtualProtect(&thunk->u1.Function, sizeof(DWORD_PTR), originalProtect, &originalProtect))
                return TRUE;
        }
    }

    return FALSE;
}