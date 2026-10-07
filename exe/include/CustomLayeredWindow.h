#ifndef INC_GDCL_EXE_CUSTOM_LAYERED_WINDOW_H
#define INC_GDCL_EXE_CUSTOM_LAYERED_WINDOW_H

#include <string>
#include <map>
#include <memory>
#include <Windows.h>
#include "CustomWidget.h"

class CustomLayeredWindow
{
    public:
        virtual ~CustomLayeredWindow()
        {
            _widgets.clear();
            _window = NULL;
            DeleteDC(_buffer);
        }

        HDC GetBuffer() { return _buffer; }
        HWND GetWindow() { return _window; }

        CustomWidget* GetWidget(std::string name) const
        {
            auto it = _widgets.find(name);
            if (it != _widgets.end())
            {
                return it->second.get();
            }
            return nullptr;
        }

        virtual void RegisterWindow(HINSTANCE instance) = 0;
        virtual void BuildWindow(HINSTANCE instance) = 0;

    protected:

        virtual void Redraw(HWND hwnd) = 0;

        HWND _window;
        HDC _buffer;
        std::map<std::string, std::unique_ptr<CustomWidget>> _widgets;
};

#endif//INC_GDCL_EXE_CUSTOM_LAYERED_WINDOW_H