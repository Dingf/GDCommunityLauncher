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
        HookManager() {}

        static HookManager& GetInstance();

        struct ExportKey
        {
            ExportKey() {}
            ExportKey(LPCSTR moduleName, LPCSTR functionName) : _module(moduleName), _function(functionName) {}

            bool operator<(const ExportKey& key) const
            {
                int compare = _module.compare(key._module);
                if (compare == 0)
                {
                    return (_function.compare(key._function) < 0);
                }
                else
                {
                    return (compare < 0);
                }
            }

            std::string _module;
            std::string _function;
        };

        std::map<ExportKey, LPVOID> _originalFunctions;
};

#endif//INC_GDCL_DLL_HOOK_MANAGER_H