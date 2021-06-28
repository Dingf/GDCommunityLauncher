#ifndef INC_GDCL_DLL_HOOK_MANAGER_H
#define INC_GDCL_DLL_HOOK_MANAGER_H

#include <string>
#include <map>
#include <Windows.h>

class HookManager
{
    public:
        static LPVOID GetOriginalFunction(LPCSTR moduleName, LPCSTR functionName);

        static LPVOID CreateHook(LPCSTR moduleName, LPCSTR functionName, PVOID function);

        static BOOL DeleteHook(LPCSTR moduleName, LPCSTR functionName);

    private:
        typedef std::pair<std::string, std::string> ExportKey;

        HookManager() {}

        static HookManager& GetInstance();

        std::map<ExportKey, LPVOID> _originalFunctions;
};

#endif//INC_GDCL_DLL_HOOK_MANAGER_H