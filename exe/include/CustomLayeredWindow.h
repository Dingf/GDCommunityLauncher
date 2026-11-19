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
            _window = NULL;
            ClearWidgets();
            DeleteDC(_buffer);
        }

        HDC GetBuffer() { return _buffer; }
        HWND GetWindow() { return _window; }

        void ClearWidgets() { _widgets.clear(); }

        template <typename T>
        T* GetWidget(std::string name) const
        {
            auto it = _widgets.find(name);
            if (it != _widgets.end())
            {
                return dynamic_cast<T*>(it->second.get());
            }
            return nullptr;
        }

        bool SendMessageToWidgets(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
        {
            bool result = false;
            for (const auto& pair : _widgets)
            {
                if (pair.second->HandleMessage(hwnd, msg, wp, lp))
                    result = true;
            }
            return result;
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