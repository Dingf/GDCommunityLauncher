#ifndef INC_GDCL_EXE_CUSTOM_BUTTON_H
#define INC_GDCL_EXE_CUSTOM_BUTTON_H

#include "CustomWidget.h"

class CustomButton : public CustomWidget
{
    public:
        CustomButton(HINSTANCE instance, HDC parent, RECT bounds, uint32_t resourceID, CustomWidgetHandler handler);

        void SetWidgetState(WidgetState state) { _state = state; Redraw(); }
        void SetFocusState(bool focus);

        void SetClickHandler(CustomWidgetHandler handler) { _clickHandler = handler; }
        CustomWidgetHandler GetClickHandler() const { return _clickHandler; }

    private:
        bool DefaultMouseMoveHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultLButtonDownHandler(HWND hwnd, UINT msg,  WPARAM wp, LPARAM lp);
        bool DefaultLButtonUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultKeyDownHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
        bool DefaultKeyUpHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

        void Redraw();

        bool _clicked;
        CustomWidgetHandler _clickHandler;
};

#endif//INC_GDCL_EXE_CUSTOM_BUTTON_H