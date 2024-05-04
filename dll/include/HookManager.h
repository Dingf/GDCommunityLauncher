#ifndef INC_GDCL_DLL_HOOK_MANAGER_H
#define INC_GDCL_DLL_HOOK_MANAGER_H

#include <string>
#include <map>
#include <Windows.h>

class HookManager
{
    public:
        struct Hook
        {
            LPCSTR moduleName;
            LPCSTR functionName;
            LPVOID function;
            bool forceMinHook;
        };

        ~HookManager();

        static LPVOID GetOriginalFunction(const Hook& hook);
        static LPVOID GetOriginalFunction(LPCSTR moduleName, LPCSTR functionName);

        static LPVOID CreateHook(const Hook& hook);
        static LPVOID CreateHook(LPCSTR moduleName, LPCSTR functionName, LPVOID function, bool forceMinHook = false);

        static BOOL DeleteHook(const Hook& hook);
        static BOOL DeleteHook(LPCSTR moduleName, LPCSTR functionName);

    private:
        typedef std::pair<std::string, std::string> ExportKey;

        HookManager();
        HookManager(HookManager&) = delete;
        void operator=(const HookManager&) = delete;

        static HookManager& GetInstance();

        std::map<ExportKey, LPVOID> _originalFunctions;
};

#endif//INC_GDCL_DLL_HOOK_MANAGER_H