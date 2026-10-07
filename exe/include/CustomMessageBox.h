#ifndef INC_GDCL_EXE_CUSTOM_MESSAGE_BOX_H
#define INC_GDCL_EXE_CUSTOM_MESSAGE_BOX_H

#include <string>
#include <Windows.h>
#include "CustomLayeredWindow.h"

#define WM_CUSTOM_MB_OK 0xA000

class CustomMessageBox : public CustomLayeredWindow
{
    public:
        ~CustomMessageBox();

        static void CreateMessageBox(HWND parent, const std::wstring& text, uint32_t sound);

    private:
        CustomMessageBox(HWND parent, const std::wstring& text);

        static INT_PTR CALLBACK MessageBoxHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void RegisterWindow(HINSTANCE instance);
        void BuildWindow(HINSTANCE instance);

        void LockParentWindow();
        void UnlockParentWindow();

        void Redraw(HWND hwnd);

        const HWND   _parent;
        std::wstring _text;
};

#endif//INC_GDCL_EXE_CUSTOM_MESSAGE_BOX_H