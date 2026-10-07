#ifndef INC_GDCL_EXE_FONT_MANAGER_H
#define INC_GDCL_EXE_FONT_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <Windows.h>
#include <gdiplus.h>

class FontManager
{
    public:
        static FontManager* GetInstance();
        ~FontManager();

        bool LoadFontFromResource(const std::string& name, HRSRC resource);

        Gdiplus::FontFamily* GetFont(const std::string& name);

    private:
        FontManager() {}

        Gdiplus::PrivateFontCollection _fontCollection;
        std::unordered_map<std::string, std::unique_ptr<Gdiplus::FontFamily>> _fonts;
};

#define spFontManager FontManager::GetInstance()

#endif//INC_GDCL_EXE_FONT_MANAGER_H