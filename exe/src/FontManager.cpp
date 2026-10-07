#include <Windows.h>
#include <gdiplus.h>
#include "FontManager.h"

FontManager::~FontManager()
{
    _fonts.clear();
}

FontManager* FontManager::GetInstance()
{
    static FontManager instance;
    return &instance;
}

bool FontManager::LoadFontFromResource(const std::string& name, HRSRC resource)
{
    if (resource)
    {
        HINSTANCE instance = GetModuleHandle(NULL);
        if (HGLOBAL data = LoadResource(instance, resource))
        {
            LPVOID buffer = LockResource(data);
            DWORD bufferSize = SizeofResource(instance, resource);
            if ((buffer != nullptr) && (bufferSize > 0))
            {
                if (_fontCollection.AddMemoryFont(buffer, bufferSize) == Gdiplus::Status::Ok)
                {
                    int32_t count = _fontCollection.GetFamilyCount();
                    int32_t found = 0;

                    Gdiplus::FontFamily* families = new Gdiplus::FontFamily[count];
                    _fontCollection.GetFamilies(count, families, &found);
                    _fonts.emplace(name, families[count-1].Clone());
                    delete[] families;
                    return true;
                }
            }
        }
    }
    return false;
}

Gdiplus::FontFamily* FontManager::GetFont(const std::string& name)
{
    auto it = _fonts.find(name);
    return (it != _fonts.end()) ? it->second.get() : nullptr;
}